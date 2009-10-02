#include <string.h>

#include "sqlite-base.hh"

#define MAX_STEPS 10000

namespace kvtest {

    PreparedStatement::PreparedStatement(sqlite3 *d, const char *query) {
        db = d;
        if(sqlite3_prepare_v2(db, query, strlen(query), &st, NULL)
           != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errmsg(db));
        }
    }

    PreparedStatement::~PreparedStatement() {
        sqlite3_finalize(st);
    }

    void PreparedStatement::bind(int pos, const char *s) {
        sqlite3_bind_text(st, pos, s, strlen(s), SQLITE_TRANSIENT);
    }

    int PreparedStatement::execute() {
        int steps_run = 0, rc = 0;
        while ((rc = sqlite3_step(st)) != SQLITE_DONE) {
            steps_run++;
            assert(steps_run < MAX_STEPS);
        }
        return sqlite3_changes(db);
    }

    bool PreparedStatement::fetch() {
        bool rv = true;
        assert(st);
        switch(sqlite3_step(st)) {
        case SQLITE_BUSY:
            throw std::runtime_error("DB was busy.");
            break;
        case SQLITE_ROW:
            break;
        case SQLITE_DONE:
            rv = false;
            break;
        default:
            throw std::runtime_error("Unhandled case.");
        }
        return rv;
    }

    const char *PreparedStatement::column(int x) {
        return (char*)sqlite3_column_text(st, x);
    }

    void PreparedStatement::reset() {
        if(sqlite3_reset(st) != SQLITE_OK) {
            throw std::runtime_error("Error resetting statement.");
        }
    }

    Sqlite3::Sqlite3(const char *fn) {
        filename = fn;
        db = NULL;
        open();
    }

    Sqlite3::~Sqlite3() {
        close();
    }

    void Sqlite3::open() {
        if(!db) {
            if(sqlite3_open(filename, &db) !=  SQLITE_OK) {
                throw std::runtime_error("Error initializing sqlite3");
            }

            intransaction = false;
            initTables();

            ins_stmt = new PreparedStatement(db, "insert into kv(k,v) values(?, ?)");
            sel_stmt = new PreparedStatement(db, "select v from kv where k = ?");
            del_stmt = new PreparedStatement(db, "delete from kv where k = ?");
        }
    }

    void Sqlite3::close() {
        if(db) {
            intransaction = false;
            delete ins_stmt;
            delete sel_stmt;
            delete del_stmt;
            ins_stmt = sel_stmt = del_stmt = NULL;
            sqlite3_close(db);
            db = NULL;
        }
    }

    void Sqlite3::initTables() {
        execute("create table if not exists kv"
                " (k varchar(250) primary key on conflict replace,"
                "  v text)");
    }

    void Sqlite3::destroyTables() {
        execute("drop table if exists kv");
    }

    void Sqlite3::reset() {
        close();
        open();
        destroyTables();
        initTables();
        execute("vacuum");
    }

    void Sqlite3::begin() {
        if(!intransaction) {
            execute("begin");
            intransaction = true;
        }
    }

    void Sqlite3::commit() {
        if(intransaction) {
            intransaction = false;
            execute("commit");
        }
    }

    void Sqlite3::rollback() {
        if(intransaction) {
            intransaction = false;
            execute("rollback");
        }
    }

    void Sqlite3::execute(const char *query) {
        PreparedStatement st(db, query);
        st.execute();
    }

    void Sqlite3::set(std::string &key, std::string &val,
                      kvtest::Callback<bool> &cb) {
        ins_stmt->bind(1, key.c_str());
        ins_stmt->bind(2, val.c_str());
        bool rv = ins_stmt->execute() == 1;
        cb.callback(rv);
        ins_stmt->reset();
    }

    void Sqlite3::get(std::string &key, kvtest::Callback<kvtest::GetValue> &cb) {
        sel_stmt->bind(1, key.c_str());

        if(sel_stmt->fetch()) {
            std::string str(sel_stmt->column(0));
            kvtest::GetValue rv(str, true);
            cb.callback(rv);
        } else {
            std::string str(":(");
            kvtest::GetValue rv(str, false);
            cb.callback(rv);
        }
        sel_stmt->reset();
    }

    void Sqlite3::del(std::string &key, kvtest::Callback<bool> &cb) {
        del_stmt->bind(1, key.c_str());
        bool rv = del_stmt->execute() == 1;
        cb.callback(rv);
        del_stmt->reset();
    }

}
