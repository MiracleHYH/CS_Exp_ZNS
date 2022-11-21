#include "spdk/stdinc.h"
#include "spdk/thread.h"
#include "spdk/bdev.h"
#include "spdk/env.h"
#include "spdk/event.h"
#include "spdk/log.h"
#include "spdk/string.h"
#include "spdk/bdev_zone.h"

static char *g_bdev_name = "Nvme0n1";

const int DATA_LENGTH = 256*1024;

struct my_context
{
    struct spdk_bdev *bdev;
    struct spdk_bdev_desc *bdev_desc;
    struct spdk_io_channel *bdev_io_channel;
    char *buff;
    uint32_t buff_size;
    char *bdev_name;
    struct spdk_bdev_io_wait_entry bdev_io_wait;
};

static char *generate_str(void)
{
    char *str = (char *)malloc(DATA_LENGTH * 8);
    memset(str, 0, DATA_LENGTH*8);
    if (str)
    {
        int i;
        for (i = 0; i < DATA_LENGTH; ++ i)
        {
            str[i] = '0'+(i%10);
        }
        return str;
    }
    else
    {
        return NULL;
    }
}

static void save_data(const char *file_path, char *str)
{
    FILE *fp = fopen(file_path, "w");
    fprintf(fp, "%s", str);
    fclose(fp);
}

static int miracle_bdev_parse_arg(int ch, char *arg)
{
    switch (ch)
    {
    case 'b':
        g_bdev_name = arg;
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

static void miracle_bdev_usage(void)
{
    printf(" -b <bdev>                 name of the bdev to use\n");
}

static void bdev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev *bdev, void *event_ctx)
{
    SPDK_NOTICELOG("Unsupported bdev event: type %d\n", type);
}

static void read_complete(struct spdk_bdev_io *bdev_io, bool success, void *cb_arg)
{
    struct my_context *p = cb_arg;

    if (success)
    {
        SPDK_NOTICELOG("Reading Successfully, Saveing to data.out\n");
        save_data("./data.out", p->buff);
    }
    else
    {
        SPDK_ERRLOG("bdev io read error\n");
    }

    spdk_bdev_free_io(bdev_io);
    spdk_put_io_channel(p->bdev_io_channel);
    spdk_bdev_close(p->bdev_desc);
    SPDK_NOTICELOG("Stopping app\n");
    spdk_app_stop(success ? 0 : -1);
}

static void start_read(void *arg)
{
    struct my_context *p = arg;
    int rc = 0;

    SPDK_NOTICELOG("Reading io\n");
    rc = spdk_bdev_read(p->bdev_desc, p->bdev_io_channel, p->buff, 0, p->buff_size, read_complete, p);

    if (rc == -ENOMEM)
    {
        SPDK_NOTICELOG("Queueing io\n");
        p->bdev_io_wait.bdev = p->bdev;
        p->bdev_io_wait.cb_fn = start_read;
        p->bdev_io_wait.cb_arg = p;
        spdk_bdev_queue_io_wait(p->bdev, p->bdev_io_channel, &p->bdev_io_wait);
    }
    else if (rc)
    {
        SPDK_ERRLOG("%s error while reading from bdev: %d\n", spdk_strerror(-rc), rc);
        spdk_put_io_channel(p->bdev_io_channel);
        spdk_bdev_close(p->bdev_desc);
        spdk_app_stop(-1);
    }
}

static void write_complete(struct spdk_bdev_io *bdev_io, bool success, void *cb_arg)
{
    struct my_context *p = cb_arg;

    spdk_bdev_free_io(bdev_io);

    if (success)
    {
        SPDK_NOTICELOG("bdev io write completed successfully\n");
    }
    else
    {
        SPDK_ERRLOG("bdev io write error: %d\n", EIO);
        spdk_put_io_channel(p->bdev_io_channel);
        spdk_bdev_close(p->bdev_desc);
        spdk_app_stop(-1);
        return;
    }

    memset(p->buff, 0, p->buff_size);
    start_read(p);
}

static void start_write(void *arg)
{
    struct my_context *p = arg;
    int rc = 0;

    SPDK_NOTICELOG("Writing to the bdev\n");
    rc = spdk_bdev_write(p->bdev_desc, p->bdev_io_channel, p->buff, 0, p->buff_size, write_complete, p);

    if (rc == -ENOMEM)
    {
        SPDK_NOTICELOG("Queueing io\n");
        p->bdev_io_wait.bdev = p->bdev;
        p->bdev_io_wait.cb_fn = start_write;
        p->bdev_io_wait.cb_arg = p;
        spdk_bdev_queue_io_wait(p->bdev, p->bdev_io_channel, &p->bdev_io_wait);
    }
    else if (rc)
    {
        SPDK_ERRLOG("%s error while writing to bdev: %d\n", spdk_strerror(-rc), rc);
        spdk_put_io_channel(p->bdev_io_channel);
        spdk_bdev_close(p->bdev_desc);
        spdk_app_stop(-1);
    }
}

static void miracle_bdev(void *arg)
{
    struct my_context *p = arg;
    uint32_t buf_align;
    uint32_t block_size;
    int rc = 0;
    p->bdev = NULL;
    p->bdev_desc = NULL;

    SPDK_NOTICELOG("Successfully started the application\n");

    SPDK_NOTICELOG("Opening the bdev %s\n", p->bdev_name);
    rc = spdk_bdev_open_ext(p->bdev_name, true, bdev_event_cb, NULL, &p->bdev_desc);
    if (rc)
    {
        SPDK_ERRLOG("Could not open bdev: %s\n", p->bdev_name);
        spdk_app_stop(-1);
        return;
    }

    p->bdev = spdk_bdev_desc_get_bdev(p->bdev_desc);

    SPDK_NOTICELOG("Opening io channel\n");
    p->bdev_io_channel = spdk_bdev_get_io_channel(p->bdev_desc);
    if (p->bdev_io_channel == NULL)
    {
        SPDK_ERRLOG("Could not create bdev I/O channel!!\n");
        spdk_bdev_close(p->bdev_desc);
        spdk_app_stop(-1);
        return;
    }

    block_size = spdk_bdev_get_block_size(p->bdev);
    buf_align = spdk_bdev_get_buf_align(p->bdev);

    p->buff_size = ceil(1.0*DATA_LENGTH/block_size)*block_size;
    p->buff = spdk_dma_zmalloc(p->buff_size, buf_align, NULL);
    if (!p->buff)
    {
        SPDK_ERRLOG("Failed to allocate buffer\n");
        spdk_put_io_channel(p->bdev_io_channel);
        spdk_bdev_close(p->bdev_desc);
        spdk_app_stop(-1);
        return;
    }

    SPDK_NOTICELOG("Generating Data\n");
    char *str = generate_str();
    if (str){
        sprintf(p->buff, "%s", str);
        free(str);
        SPDK_NOTICELOG("Saving Data to ./data.in\n");
        save_data("./data.in", p->buff);
        start_write(p);
    }
    else{
        SPDK_ERRLOG("Could not generate data!!\n");
        spdk_put_io_channel(p->bdev_io_channel);
        spdk_bdev_close(p->bdev_desc);
        spdk_app_stop(-1);
        return;
    }
}

int main(int argc, char **argv)
{
    struct spdk_app_opts opts = {};
    int rc = 0;
    struct my_context context = {};

    spdk_app_opts_init(&opts, sizeof(opts));
    opts.name = "miracle_bdev";
    rc = spdk_app_parse_args(argc, argv, &opts, "b:", NULL, miracle_bdev_parse_arg, miracle_bdev_usage);
    if (rc != SPDK_APP_PARSE_ARGS_SUCCESS)
    {
        exit(rc);
    }
    context.bdev_name = g_bdev_name;

    rc = spdk_app_start(&opts, miracle_bdev, &context);
    if (rc)
    {
        SPDK_ERRLOG("ERROR starting applicatoin\n");
    }

    spdk_dma_free(context.buff);
    spdk_app_fini();

    return rc;
}
