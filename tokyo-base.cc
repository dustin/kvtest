#include <stdio.h>
#include <string.h>
#include <iostream>

#include "base-test.hh"
#include "suite.hh"
#include "tokyo-base.hh"

using namespace std;
using namespace kvtest;

TokyoStore::TokyoStore(const char *p, bool should_autocommit) {
    path = p;
    hdb = NULL;
    ecode = 0;
    autocommit = should_autocommit;
    open();
}

void TokyoStore::reset() {
    close();
    if(access(path, R_OK) == 0 && unlink(path) != 0) {
        throw std::runtime_error("Failed to unlink database.");
    }
    open();
}

void TokyoStore::set(std::string &key, std::string &val,
                   Callback<bool> &cb) {
  bool rv = true;
  //int ecode;

  if (!tchdbput2(hdb, key.c_str(), val.c_str())) {
    /*
      ecode = tchdbecode(hdb);
      char errmsg[ERRSTR_SIZE];
      ssprintf(errmsg, ERRSTR_SIZE, "put error: %s\n", tchdberrmsg(ecode));
    */
    rv= false;
  }

  cb.callback(rv);
}

void TokyoStore::get(std::string &key, Callback<GetValue> &cb) {
  //int ecode;
  char *value;

  /* retrieve data */
  value = tchdbget2(hdb, key.c_str());
  if (value) {
    std::string str(value);
    kvtest::GetValue rv(str, true);
    cb.callback(rv);
    free(value);
  }
  else {
    /*
    char errmsg[64];
    ecode = tchdbecode(hdb);
    snprintf(errmsg, 64, "get error: %s\n", tchdberrmsg(ecode));
   */
    std::string str(":(");
    kvtest::GetValue rv(str, false);
    cb.callback(rv);
  }
}

void TokyoStore::del(std::string &key, Callback<bool> &cb) {
  bool rv = true;
  if (!tchdbout(hdb, key.c_str(), key.length()))
  {
    rv = false;
  }

  cb.callback(rv);
}

void TokyoStore::open() {
    int ecode;
    int flags = HDBOWRITER | HDBOCREAT;

    if (!hdb) {
        hdb = tchdbnew();
        if (hdb == NULL) {
            throw std::runtime_error("error creating tchdb instance.");
        }


        /* open the database */
        if (!tchdbopen(hdb, path, flags)) {
          char msg[ERRSTR_SIZE];
          ecode = tchdbecode(hdb);
          snprintf(msg, ERRSTR_SIZE, "open tchdb error: %s\n", tchdberrmsg(ecode));
          throw std::runtime_error(msg);
        }
    }
}

void TokyoStore::close() {
  //int ecode;
  if (!tchdbclose(hdb)){
    /*
    char errmsg[ERRSTR_SIZE];
    ecode = tchdbecode(hdb);
    snprintf(errmsg, ERRSTR_SIZE, "close error: %s\n", tchdberrmsg(ecode));
    */
  }
  tchdbdel(hdb);
  hdb = NULL;
}

void TokyoStore::commit() {
  tchdbsync(hdb);
  // or should it be this?
  //bool tchdbtrancommit(TCHDB *hdb);
}
