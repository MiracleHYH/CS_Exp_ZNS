#ifndef BITCASK_H_
#define BITCASK_H_

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <pwd.h>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#include <boost/crc.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

using namespace std;

#define filemax 1024 * 4096 // 4MB

const string fileprev = "bitcask_data";
const string cmd_prompt = ">>> bitcask : ";
const string cmd = ">>> ";
const int number = 0;
// data
struct bitcask_data
{
    string key;
    int key_len;
    string value;
    int value_len;
    boost::posix_time::ptime timestamp;
    uint32_t crc;
    // crc
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar &key_len;
        ar &key;
        ar &value_len;
        ar &value;
        ar &crc;
        ar &timestamp;
    }
};

// index
struct bitcask_index
{
    string key;
    string file_id;
    int value_pos;
    int value_len;
    boost::posix_time::ptime timestamp;
    bool value_valid;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar &key;
        ar &file_id;
        ar &value_pos;
        ar &value_len;
        ar &timestamp;
        ar &value_valid;
    }

    bitcask_index()
    {
        value_valid = false;
    }
};

// bitcask
class bitcask
{
private:
    unordered_map<string, bitcask_index> index;
    int _activefile;
    bool _start;
    bool _finish;
    string _response;
    string filepath;

private:
    void init(string path);
    uint32_t crc32(string value);
    void insert_data(string key, string value);
    void write_data(bitcask_data newdata);
    void write_index(bitcask_index newindex);
    bitcask_data read_data(string key);
    void read_datainfo(string key);
    bitcask_index read_index(string key);
    void delete_data(string key);
    void update_data(string key, string value);
    void update_index(bitcask_index upindex, string key);
    void merge();
    void flush(); // flush index :hint.bin

public:
    bitcask();
    string Get(string key);
    void Put(string key, string value);
    void Open(string path);
    void Close();
    ~bitcask();

};

#endif /* BITCASK_H_ */
