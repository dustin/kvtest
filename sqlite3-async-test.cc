#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <queue>

#include "base-test.hh"
#include "suite.hh"
#include "sqlite-base.hh"

using namespace kvtest;

class DBOperation {
public:
    virtual bool execute(Sqlite3 *db) {}
};

class ResetOperation : public DBOperation {
public:

    ResetOperation(Callback<bool> *c) {
        cb = c;
    }

    bool execute(Sqlite3 *db) {
        db->rollback();
        db->reset();
        bool rv = true;
        cb->callback(rv);
    }

private:
    Callback<bool> *cb;
};

class NOOPOperation : public DBOperation {
public:

    NOOPOperation(Callback<bool> *c) {
        cb = c;
    }

    bool execute(Sqlite3 *db) {
        bool rv = true;
        cb->callback(rv);
    }

private:
    Callback<bool> *cb;
};

class SetOperation : public DBOperation {
public:

    SetOperation(std::string &k, std::string &v,
                 Callback<bool> *c) {
        key = k;
        value = v;
        cb = c;
    }

    bool execute(Sqlite3 *db) {
        db->set(key, value, *cb);
    }

private:
    std::string     key;
    std::string     value;
    Callback<bool> *cb;
};

class GetOperation : public DBOperation {
public:

    GetOperation(std::string &k, Callback<GetValue> *c) {
        key = k;
        cb = c;
    }

    bool execute(Sqlite3 *db) {
        db->get(key, *cb);
    }

private:
    std::string                         key;
    Callback<GetValue> *cb;
};

class DeleteOperation : public DBOperation {
public:

    DeleteOperation(std::string &k, Callback<bool> *c) {
        key = k;
        cb = c;
    }

    bool execute(Sqlite3 *db) {
        db->del(key, *cb);
    }

private:
    std::string             key;
    Callback<bool> *cb;
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
        LockHolder lh(&mutex);
        ops.push(op);
        if(pthread_cond_signal(&cond) != 0) {
            throw std::runtime_error("Error signaling change.");
        }
    }

    void drainTo(std::queue<DBOperation*> &out) {
        LockHolder lh(&mutex);
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

class QueuedSqlite : public ThingUnderTest {
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

    void reset() {
        std::queue<DBOperation*> opq;
        RememberingCallback<bool> cb;
        iq->addOperation(new ResetOperation(&cb));
        cb.waitForValue();
    }

    void set(std::string &key, std::string &val,
             Callback<bool> &cb) {
        iq->addOperation(new SetOperation(key, val, &cb));
    }

    void get(std::string &key, Callback<GetValue> &cb) {
        iq->addOperation(new GetOperation(key, &cb));
    }

    void del(std::string &key, Callback<bool> &cb) {
        iq->addOperation(new DeleteOperation(key, &cb));
    }

    void noop(Callback<bool> &cb) {
        iq->addOperation(new NOOPOperation(&cb));
    }

private:
    Sqlite3       *db;
    InboundQueue  *iq;
    QueryExecutor *executor;
    pthread_t      thread;
};

int main(int argc, char **args) {
    QueuedSqlite *thing = new QueuedSqlite("/tmp/test.db");

    TestSuite suite(thing);
    return suite.run() ? 0 : 1;
}
