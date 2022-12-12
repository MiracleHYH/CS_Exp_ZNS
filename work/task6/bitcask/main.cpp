#include "bitcask.h"

#define db_path "/home/miracle/work/task6/bitcask/db/"

int main()
{
    bitcask *db = new bitcask;
    db->Open(db_path);

    db->Put("key1", "value1");
    cout << db->Get("key1") << endl;
    db->Close();
    cout << db->Get("key1") << endl;
    db->Open(db_path);
    cout << db->Get("key1") << endl;
    cout << db->Get("key2") << endl;
    return 0;
}
