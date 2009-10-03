#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "base-test.hh"
#include "suite.hh"
#include "bdb-base.hh"

using namespace std;
using namespace kvtest;

BDBStore::BDBStore(const char *p, bool should_autocommit) {
    path = p;
    autocommit = should_autocommit;
    db = NULL;
    open();
}

void BDBStore::reset() {
    close();
    if(access(path, R_OK) == 0 && unlink(path) != 0) {
        throw std::runtime_error("Failed to unlink database.");
    }
    open();
}

void BDBStore::set(std::string &key, std::string &val,
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
    if (autocommit) {
        db->sync(db, 0);
    }
    cb.callback(rv);
}

void BDBStore::get(std::string &key, Callback<GetValue> &cb) {
    DBT bdbkey, bdbdata;

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

void BDBStore::del(std::string &key, Callback<bool> &cb) {
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

void BDBStore::open() {
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

void BDBStore::close() {
    if (db) {
        db->close(db, 0);
        db = NULL;
    }
}

void BDBStore::commit() {
    db->sync(db, 0);
}
