    #include <iostream>
    #include <fstream>
    #include <string>
    #include <ctime>
    #include <unistd.h>
    #include <dirent.h>
    #include <pwd.h>
    #include "bitcask.h"
    using namespace std;

    bitcask::bitcask()
    {
        _start = false;
        _activefile = 0;
        _finish = false;
        _response = "";
        filepath = "";
    }

    bitcask::~bitcask()
    {
        if (this->_start)
        {
            merge();
            flush();
        }
    }

    void bitcask::init(string path)
    {
        this->filepath = path;
        this->_start = true;
        long len;
        fstream hint;
        hint.open(filepath + "hint.bin", ios::binary | ios::out | ios::app);
        if (!hint)
        {
            cout << "the file hint.bin open failure or maybe not exist!\n";
        }
        len = hint.tellg();
        if (len == 0)
        {
            cout << "create file hint.bin successful!\n";
        }
        else
        {
            /*
            load index to memory
            */
            // cout << "loading index" << endl;
            bitcask_index search;
            fstream hint;
            hint.open(filepath + "hint.bin", ios::binary | ios::in);
            if (!hint)
            {
                cout << "the file hint.bin open failure or maybe not exist!\n";
            }

            while (hint)
            {
                boost::archive::binary_iarchive ia(hint, boost::archive::no_header);
                try
                {
                    ia >> search;
                }
                catch (const exception &e)
                {
                    // cout << "read end" << endl;
                    goto do_load;
                }
                // cout << "loading:" << search.key << endl;
                bitcask_index insert;
                insert.file_id = search.file_id;
                insert.value_pos = search.value_pos;
                insert.value_len = search.value_len;
                insert.timestamp = search.timestamp;
                insert.value_valid = search.value_valid;
                index[search.key] = insert;
            }
        }
    do_load:
        fstream filelog;
        filelog.open(filepath + "filelog.bin", ios::binary | ios::in);
        if (!filelog)
        {
            cout << "the file filelog.bin open failure or maybe not exist!\n";
        }
        filelog.read((char *)(&_activefile), sizeof(int));
        filelog.close();
        if (_activefile == 0)
        {
            cout << "create file filelog.bin successful!\n";
            _activefile = 1;
            filelog.open(filepath + "filelog.bin", ios::binary | ios::out | ios::app);
            filelog.write((char *)(&_activefile), sizeof(int));
            filelog.close();
            return;
        }
        _start = true;
    }

    uint32_t bitcask::crc32(string value)
    {
        boost::crc_32_type result;
        result.process_bytes(value.c_str(), value.length());
        return result.checksum();
    }

    void bitcask::insert_data(string key, string value)
    {
        bitcask_index search = read_index(key);
        if (search.key != "")
        {
            cout << "the data " + key + " already exist!\n";
            return update_data(key, value);
        }
        // add data
        bitcask_data newdata;
        newdata.key = key;
        newdata.key_len = int(key.length());
        newdata.value = value;
        newdata.value_len = int(value.length());
        newdata.crc = crc32(value);
        // newdata.timestamp=time(0);
        newdata.timestamp = boost::posix_time::microsec_clock::universal_time();

        // add index
        fstream datafile;
        bitcask_index newindex;
        newindex.key = key;
        newindex.file_id = fileprev + to_string(_activefile);
        datafile.open(filepath + newindex.file_id, ios::binary | ios::out | ios::app);
        if (!datafile)
            cout << cmd_prompt + newindex.file_id + " open failure\n";
        newindex.value_pos = datafile.tellg();
        if (newindex.value_pos > filemax || filemax - newindex.value_pos < sizeof(newdata))
        {
            _activefile++;
            newindex.file_id = fileprev + to_string(_activefile);
        }
        newindex.timestamp = newdata.timestamp;
        newindex.value_len = sizeof(newdata);
        newindex.value_valid = true;
        datafile.close();
        write_data(newdata);
        write_index(newindex);
        // add to memory index array
        index[key] = newindex;
        // cout << "the data " + key + " insert successful\n";
    }

    void bitcask::write_data(bitcask_data newdata)
    {
        string file = fileprev + to_string(_activefile);
        fstream datafile;
        datafile.open(filepath + file, ios::binary | ios::out | ios::app);
        if (!datafile)
            cout << file + " open file " + file + " failure!\n";
        // datafile.write((char *)(&newdata),sizeof(newdata));
        // data_append(datafile, newdata);
        boost::archive::binary_oarchive oa(datafile, boost::archive::no_header);
        oa << newdata;
        datafile.close();
    }

    void bitcask::write_index(bitcask_index newindex)
    {
        fstream hint;
        hint.open(filepath + "hint.bin", ios::binary | ios::out | ios::app);
        if (!hint)
        {
            cout << "the file hint.bin open failure!\n";
        }
        // hint.write((char *)(&newindex), sizeof(newindex));
        // cout << "writing index: " << newindex.key << newindex.timestamp << endl;
        boost::archive::binary_oarchive oa(hint, boost::archive::no_header);
        oa << newindex;
        hint.close();
    }

    bitcask_index bitcask::read_index(string key)
    {

        bitcask_index search;
        if ("" == key)
        {
            return search;
        }
        for (auto indexinfo : index)
        {
            if (indexinfo.first == key)
            {
                {

                    search.key = indexinfo.first;
                    search.file_id = indexinfo.second.file_id;
                    search.value_pos = indexinfo.second.value_pos;
                    search.value_len = indexinfo.second.value_len;
                    search.value_valid = indexinfo.second.value_valid;
                    search.timestamp = indexinfo.second.timestamp;
                    // cout << "got key for " << key << " ,valid " << search.value_valid << endl;
                    return search;
                }
            }
        }
        return search;
    }

    bitcask_data bitcask::read_data(string key)
    {
        bitcask_data search_data;
        bitcask_index search_index = read_index(key);
        if (search_index.value_valid == true)
        {
            string file = search_index.file_id;
            // cout << "reading file" << file << endl;
            fstream datafile;
            datafile.open(filepath + file, ios::binary | ios::in);
            if (!datafile)
                cout << "open file " + file + " failure\n";

            boost::archive::binary_iarchive ia(datafile, boost::archive::no_header);
            if (search_index.value_pos)
            {
                datafile.seekg(search_index.value_pos, ios::beg);
                // cout << "seeking to" << search_index.value_pos << endl;
            }
            // datafile.read((char *)(&search_data),sizeof(search_data));
            // cout << "reading datafile" << filepath + file << endl;
            // cout << "key = " << search_index.key << endl;
            // cout << "value_pos = " << search_index.value_pos << endl;
            // cout << "ts = " << search_index.timestamp << endl;
            // cout << "file pos = " << datafile.tellp() << endl;
            // search_data.serialize(ia, 0);
            ia >> search_data;
            // cout << "file pos = " << datafile.tellp() << endl;
            // cout << "on-disk ts = " << search_data.timestamp << endl;
            // return search_data;
        }
        // else
        //     cout << "the data " + key + " does not exist!\n";
        return search_data;
    }

    void bitcask::read_datainfo(string key)
    {
        bitcask_data data = read_data(key);
        bitcask_index index = read_index(key);
        if (index.value_valid == true)
        {
            _response += cmd + "key :" + key + "\n";
            _response += cmd + "value :" + data.value + "\n";
            _response += cmd + "crc :" + to_string(data.crc) + "\n";
            _response += cmd + "file id :" + index.file_id + "\n";
            _response += cmd + "value pos :" + to_string(index.value_pos) + "\n";
            _response += cmd + "value length :" + to_string(data.value_len) + "\n";
            // cout << _response;
            //_response+=cmd+"time :"+ data.timestamp;
            //_response+=cmd+"time :"+ctime(&data.timestamp);
        }
        else
            return;
    }

    void bitcask::delete_data(string key)
    {
        bitcask_index delindex = read_index(key);
        if (delindex.key != "")
        {
            delindex.value_valid = false;
            index[key] = delindex;
            cout << "the data " + key + " already delete!\n";
        }
        else
            _response += cmd_prompt + "the data " + key + " does not exist!\n";
    }

    void bitcask::update_data(string key, string value)
    {

        bitcask_index search = read_index(key);
        if (search.key == "")
        {
            cout << "the data " + key + " does not exist!\n";
            return;
        }
        // update data
        fstream datafile, hintfile;
        bitcask_data updata;
        bitcask_index upindex = read_index(key);
        updata.key = key;
        updata.key_len = int(key.length());
        updata.value = value;
        updata.value_len = int(value.length());
        // updata.timestamp=time(0);
        updata.timestamp = boost::posix_time::microsec_clock::universal_time();

        // update index
        upindex.file_id = fileprev + to_string(_activefile);
        datafile.open(filepath + upindex.file_id, ios::binary | ios::in | ios::app);
        if (!datafile)
            _response += cmd_prompt + "the file " + upindex.file_id + " open failure!\n";
        upindex.value_pos = datafile.tellg();
        if (upindex.value_pos > filemax || filemax - upindex.value_pos < sizeof(updata))
        {
            _activefile++;
        }
        upindex.timestamp = updata.timestamp;
        upindex.value_len = sizeof(updata);
        upindex.value_valid = true;
        datafile.close();
        write_data(updata);
        update_index(upindex, key);
        _response += cmd_prompt + "the data " + key + " update successful\n";
    }

    void bitcask::update_index(bitcask_index upindex, string key)
    {
        bitcask_index seaindex = read_index(key);
        seaindex.key = key;
        seaindex.file_id = upindex.file_id;
        seaindex.value_pos = upindex.value_pos;
        seaindex.value_len = upindex.value_len;
        seaindex.value_valid = upindex.value_valid;
        seaindex.timestamp = upindex.timestamp;
        index[key] = seaindex;
    }

    void bitcask::merge()
    {
        /*
        merge data in file
        function: delete data in file
        */
        int beans = 1;
        long value_pos;
        vector<bitcask_data> data_array;
        // cout << "merge begin: activefile" << _activefile << endl;
        for (; beans <= _activefile; beans++)
        {
            
            string file = fileprev + to_string(beans);
            fstream datafile;
            datafile.open(filepath + file, ios::binary | ios::in);
            
            if (!datafile)
            {
                _response += cmd_prompt + "the data file " + file + " open failure!\n";
            }
            bitcask_data beans_data;
            bitcask_index beans_index;
            datafile.seekg(0, ios::beg);
            // load to memory
            while (datafile)
            {
                boost::archive::binary_iarchive ia(datafile, boost::archive::no_header);
                try
                {
                    ia >> beans_data;

                    beans_index = read_index(beans_data.key);
                    // cout << "ts:" << beans_data.timestamp << " vs " << beans_index.timestamp << endl;
                }
                catch (const exception &e)
                {
                    goto do_merge;
                }

                if (beans_index.value_valid == true && beans_data.timestamp == beans_index.timestamp)
                {
                    // cout << "pushing:" << beans_index.key << endl;
                    data_array.push_back(beans_data);
                }
            }

        do_merge:
            for (auto data : data_array)
            {
                // cout << "dataarray = " << data.key << endl;
            }

            datafile.close();
            // write to file
            datafile.open(filepath + file, ios::binary | ios::out);
            if (!datafile)
            {
                _response += cmd_prompt + "the data file " + file + " open failure!\n";
            }
            datafile.seekg(0, ios::beg);

            for (auto data : data_array)
            {
                value_pos = datafile.tellg();
                // datafile.write((char *)(&data),sizeof(data));
                boost::archive::binary_oarchive oa(datafile, boost::archive::no_header);
                oa << data;
                bitcask_index seaindex = read_index(data.key);
                seaindex.value_pos = value_pos;
                index[data.key] = seaindex;
            }
            datafile.close();
            data_array.clear();
        }
        /*
        TODO merge file
        */
        if (_activefile >= 2)
        {
            while (_activefile > 1)
            {
                string file = fileprev + to_string(_activefile);
                fstream datafile;
                datafile.open(filepath + file, ios::binary | ios::in);
                if (!datafile)
                {
                    _response += cmd_prompt + "the data file " + file + " open failure!\n";
                }
                bitcask_data beans_data;
                bitcask_index beans_index;
                datafile.seekg(0, ios::beg);
                // TODO load to memory maybe too big?
                while (datafile)
                {
                    boost::archive::binary_iarchive ia(datafile, boost::archive::no_header);
                    try
                    {
                        ia >> beans_data;
                    }
                    catch (const exception &e)
                    {
                        goto do_merge_2;
                    }

                    data_array.push_back(beans_data);
                }

            do_merge_2:
                datafile.close();
                // write to file
                for (int pos = 1; pos < _activefile; pos++)
                {
                    string mergefile = fileprev + to_string(pos);
                    fstream datafile;
                    datafile.open(filepath + mergefile, ios::binary | ios::out | ios::app);
                    if (!datafile)
                    {
                        _response += cmd_prompt + "the data file " + file + " open failure!\n";
                    }
                    long mergefile_end = datafile.tellg();
                    bitcask_data merge_data = data_array.back();
                    bitcask_index merge_index = read_index(merge_data.key);
                    while (mergefile_end < filemax && (filemax - mergefile_end) < sizeof(merge_data))
                    {
                        // datafile.write((char *)(&merge_data), sizeof(merge_data));
                        boost::archive::binary_oarchive oa(datafile, boost::archive::no_header);
                        oa << merge_data;
                        merge_data = data_array.back();
                        merge_index = read_index(merge_data.key);
                        merge_index.value_pos = mergefile_end;
                        merge_index.file_id = mergefile;
                        index[merge_data.key] = merge_index;
                        mergefile_end += sizeof(merge_data);
                        data_array.pop_back();
                        merge_data = data_array.back();
                    }
                    datafile.close();
                }
                if (data_array.size() == 0)
                {
                    _activefile--;
                }
                else
                {
                    fstream newdatafile;
                    newdatafile.open(filepath + file, ios::binary | ios::out);
                    if (!newdatafile)
                    {
                        _response += cmd_prompt + "the data file " + file + " open failure!\n";
                    }
                    newdatafile.seekg(0, ios::beg);
                    for (auto data : data_array)
                    {
                        // newdatafile.write((char *)(&data), sizeof(data));
                        boost::archive::binary_oarchive oa(newdatafile, boost::archive::no_header);
                        oa << data;
                    }
                    newdatafile.close();
                    break;
                }
            }
        }
    }

    void bitcask::flush()
    {
        // write index file to index file hint.bin
        fstream hint;
        hint.open(filepath + "hint.bin", ios::binary | ios::out);
        if (!hint)
        {
            _response += cmd_prompt + "the file hint.bin open failure!\n";
        }
        hint.seekg(0, ios::beg);
        for (auto indexinfo : index)
        {
            if (indexinfo.second.value_valid == true)
            {
                // hint.write((char *)(&indexinfo.second), sizeof(indexinfo.second));
                boost::archive::binary_oarchive oa(hint, boost::archive::no_header);
                oa << indexinfo.second;
            }
        }
        // hint.close();
        // write active file number to file filelog.bin
        fstream filelog;
        filelog.open(filepath + "filelog.bin", ios::binary | ios::out);
        if (!filelog)
        {
            _response += cmd_prompt + "the filelog.bin open failuer!\n";
        }
        filelog.write((char *)(&_activefile), sizeof(int));
    }

    string bitcask::Get(string key)
    {
        if (this->_start == true)
        {
            bitcask_data bc_data = read_data(key);
            if (bc_data.key.length() == 0)
                cout << key + " does not exist!" << endl;
            return bc_data.value;
        }
        else
        {
            cout << "please open a database first" << endl;
            return "";
        }
    }

    void bitcask::Put(string key, string value)
    {
        if (this->_start == true)
            insert_data(key, value);
        else
            cout << "please open a database first" << endl;
    }

    void bitcask::Open(string path)
    {
        if (this->_start == false)
            init(path);
        else
            cout << "already open a database, please close first" << endl;
    }

    void bitcask::Close()
    {
        if (this->_start)
        {
            // merge();
            flush();
            _start = false;
            _activefile = 0;
            _finish = false;
            _response = "";
            filepath = "";
        }
        else
            cout << "please open a database first" << endl;
    }
