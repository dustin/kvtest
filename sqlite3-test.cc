#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <queue>

#include <sqlite3.h>

#include "base-test.hh"

#define MAX_STEPS 10000

class PreparedStatement {
public:
    PreparedStatement(sqlite3 *d, const char *query) {
        db = d;
        if(sqlite3_prepare_v2(db, query, strlen(query), &st, NULL)
           != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errmsg(db));
        }
    }

    ~PreparedStatement() {
        sqlite3_finalize(st);
    }

    void bind(int pos, const char *s) {
        sqlite3_bind_text(st, pos, s, strlen(s), SQLITE_TRANSIENT);
    }

    int execute() {
        int steps_run = 0, rc = 0;
        while ((rc = sqlite3_step(st)) != SQLITE_DONE) {
            steps_run++;
            assert(steps_run < MAX_STEPS);
        }
        return sqlite3_changes(db);
    }

    bool fetch() {
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

    const char *column(int x) {
        return (char*)sqlite3_column_text(st, x);
    }

private:
    sqlite3      *db;
    sqlite3_stmt *st;
};

class Sqlite3 : public kvtest::ThingUnderTest {
public:

    Sqlite3(const char *fn) {
        filename = fn;
        open();
    }

    ~Sqlite3() {
        close();
    }

    void open() {
        if(!db) {
            if(sqlite3_open(filename, &db) !=  SQLITE_OK) {
                throw std::runtime_error("Error initializing sqlite3");
            }

            initTables();
        }
    }

    void close() {
        if(db) {
            sqlite3_close(db);
            db = NULL;
        }
    }

    void initTables() {
        execute("create table if not exists kv"
                " (k varchar(250) primary key on conflict replace,"
                "  v text)");
    }

    void destroyTables() {
        execute("drop table if exists kv");
    }

    void reset() {
        close();
        open();
        destroyTables();
        initTables();
        execute("vacuum");
    }

    void begin() {
        execute("begin");
    }

    void commit() {
        execute("commit");
    }

    void rollback() {
        execute("rollback");
    }

    void set(std::string &key, std::string &val,
             kvtest::Callback<bool> &cb) {
        PreparedStatement st(db, "insert into kv(k,v) values(?, ?)");
        st.bind(1, key.c_str());
        st.bind(2, val.c_str());
        bool rv = st.execute() == 1;
        std::cout << "Calling callback on " << &cb << std::endl;
        cb.callback(rv);
    }

    void get(std::string &key, kvtest::Callback<kvtest::GetValue> &cb) {
        PreparedStatement st(db, "select v from kv where k = ?");
        st.bind(1, key.c_str());

        if(st.fetch()) {
            std::string str(st.column(0));
            kvtest::GetValue rv(str, true);
            cb.callback(rv);
        } else {
            std::string str(":(");
            kvtest::GetValue rv(str, false);
            cb.callback(rv);
        }
    }

    void del(std::string &key, kvtest::Callback<bool> &cb) {
        PreparedStatement st(db, "delete from kv where k = ?");
        st.bind(1, key.c_str());
        bool rv = st.execute() == 1;
        cb.callback(rv);
    }

protected:
    void execute(const char *query) {
        PreparedStatement st(db, query);
        st.execute();
    }

private:
    const char *filename;
    sqlite3 *db;
};

class DBOperation {
public:
    virtual bool execute(Sqlite3 *db) {};
};

class SetOperation : public DBOperation {
public:

    SetOperation(std::string &k, std::string &v,
                 kvtest::Callback<bool> *c) {
        key = k;
        value = v;
        cb = c;
    }

    bool execute(Sqlite3 *db) {
        db->set(key, value, *cb);
    }

private:
    std::string             key;
    std::string             value;
    kvtest::Callback<bool> *cb;
};

class GetOperation : public DBOperation {
public:

    GetOperation(std::string &k, kvtest::Callback<kvtest::GetValue> *c) {
        key = k;
        cb = c;
    }

    bool execute(Sqlite3 *db) {
        db->get(key, *cb);
    }

private:
    std::string                         key;
    kvtest::Callback<kvtest::GetValue> *cb;
};

class DeleteOperation : public DBOperation {
public:

    DeleteOperation(std::string &k, kvtest::Callback<bool> *c) {
        key = k;
        cb = c;
    }

    bool execute(Sqlite3 *db) {
        db->del(key, *cb);
    }

private:
    std::string             key;
    kvtest::Callback<bool> *cb;
};

class InboundQueue {
public:
    InboundQueue() {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
    }

    ~InboundQueue() {
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
    }

    void addOperation(DBOperation *op) {
        kvtest::LockHolder lh(&mutex);
        ops.push(op);
        if(pthread_cond_signal(&cond) != 0) {
            throw std::runtime_error("Error signaling change.");
        }
    }

    void drainTo(std::queue<DBOperation*> &out) {
        kvtest::LockHolder lh(&mutex);
        if(ops.empty()) {
            if(pthread_cond_wait(&cond, &mutex) != 0) {
                throw std::runtime_error("Error waiting for signal.");
            }
        }
        while(!ops.empty()) {
            out.push(ops.front());
            ops.pop();
        }
    }

private:
    pthread_mutex_t          mutex;
    pthread_cond_t           cond;
    std::queue<DBOperation*> ops;
};

class QueryExecutor {
public:
    QueryExecutor(Sqlite3 *d, InboundQueue *q) {
        db = d;
        iq = q;
        running = true;
    }

    void run() {
        while(running) {
            std::queue<DBOperation*> ops;
            iq->drainTo(ops);
            db->begin();
            int count = 0;

            while(!ops.empty()) {
                DBOperation *op = ops.front();
                ops.pop();

                op->execute(db);
                count++;
            }

            db->commit();
            std::cout << "Executed " << count << " operations" << std::endl;
        }
    }

private:
    bool          running;
    Sqlite3      *db;
    InboundQueue *iq;
};

static void* launch_executor_thread(void* arg) {
    QueryExecutor *executor = (QueryExecutor*) arg;
    try {
        executor->run();
    } catch(...) {
        std::cerr << "Caught a fatal exception in the thread" << std::endl;
    }
}

class QueuedSqlite : public kvtest::ThingUnderTest {
public:

    QueuedSqlite(const char *path) {
        db = new Sqlite3(path);
        iq = new InboundQueue();
        executor = new QueryExecutor(db, iq);

        if(pthread_create(&thread, NULL, launch_executor_thread, executor)
           != 0) {
            throw std::runtime_error("Error initializing queue thread");
        }
    }

    ~QueuedSqlite() {
        delete db;
        delete iq;
        delete executor;
    }

    void set(std::string &key, std::string &val,
             kvtest::Callback<bool> &cb) {
        iq->addOperation(new SetOperation(key, val, &cb));
    }

    void get(std::string &key, kvtest::Callback<kvtest::GetValue> &cb) {
        iq->addOperation(new GetOperation(key, &cb));
    }

    void del(std::string &key, kvtest::Callback<bool> &cb) {
        iq->addOperation(new DeleteOperation(key, &cb));
    }

private:
    Sqlite3       *db;
    InboundQueue  *iq;
    QueryExecutor *executor;
    pthread_t      thread;
};

int main(int argc, char **args) {
    QueuedSqlite *thing = new QueuedSqlite("/tmp/test.db");

    kvtest::TestSuite suite(thing);
    return suite.run() ? 0 : 1;
}
