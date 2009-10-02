#include <stdio.h>
#include <string.h>
#include <iostream>

#define HAVE_CXX_STDHEADERS 1
#include <db.h>

#include "base-test.hh"
#include "suite.hh"

using namespace std;
using namespace kvtest;

class BDBStore : public KVStore {
public:

    BDBStore(const char *p) {
        path = p;
        open();
    }

    ~BDBStore() {
        close();
    }

    void reset() {
        close();
        if(access(path, R_OK) == 0 && unlink(path) != 0) {
            throw std::runtime_error("Failed to unlink database.");
        }
        open();
    }

    void set(std::string &key, std::string &val,
             Callback<bool> &cb) {

        DBT bdbkey, bdbdata;
        memset(&bdbkey, 0, sizeof(DBT));
        memset(&bdbdata, 0, sizeof(DBT));

        bdbkey.data = (void*)key.c_str();
        bdbkey.size = key.length();

        bdbdata.data = (void*)val.c_str();
        bdbdata.size = val.length() + 1;

        int ret = db->put(db, NULL, &bdbkey, &bdbdata, 0);
        bool rv = ret == 0;
        db->sync(db, 0);
        cb.callback(rv);
    }

    void get(std::string &key, Callback<GetValue> &cb) {
        DBT bdbkey, bdbdata;
        char charval[8192];

        /* Zero out the DBTs before using them. */
        memset(&bdbkey, 0, sizeof(DBT));
        memset(&bdbdata, 0, sizeof(DBT));

        bdbkey.data = (void*)key.c_str();
        bdbkey.size = key.length();

        bdbdata.ulen = 1*1024*1024;
        bdbdata.flags = DB_DBT_MALLOC;
        int ret = db->get(db, NULL, &bdbkey, &bdbdata, 0);

        if (ret == 0) {
            std::string str((char *)bdbdata.data);
            kvtest::GetValue rv(str, true);
            cb.callback(rv);
            free(bdbdata.data);
        } else {
            std::string str(":(");
            kvtest::GetValue rv(str, false);
            cb.callback(rv);
        }
    }

    void del(std::string &key, Callback<bool> &cb) {
        DBT bdbkey;
        memset(&bdbkey, 0, sizeof(DBT));

        bdbkey.data = (void*)key.c_str();
        bdbkey.size = key.length();

        bool rv = true;
        if (db->del(db, NULL, &bdbkey, 0) != 0) {
            rv = false;
        }

        cb.callback(rv);
    }

private:
    DB *db;
    const char *path;

    void open() {
        if(!db) {
            int ret = db_create(&db, NULL, 0);
            if (ret != 0) {
                throw std::runtime_error("error creating bdb instance.");
            }

            int flags = DB_CREATE;

            /* open the database */
            ret = db->open(db,
                           NULL,
                           path,
                           NULL,
                           DB_BTREE,
                           flags,
                           0);
            if (ret != 0) {
                throw std::runtime_error("Error opening DB.");
            }
        }
    }

    void close() {
        if (db) {
            db->close(db, 0);
            db = NULL;
        }
    }


};

int main(int argc, char **args) {
    BDBStore *thing = new BDBStore("/tmp/test.bdb");

    TestSuite suite(thing);
    return suite.run() ? 0 : 1;
}
