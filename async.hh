#ifndef ASYNC_HH
#define ASYNC_HH 1

#include <stdlib.h>
#include <queue>

#define MAX_DRAIN 25000

namespace kvtest {

    /**
     * Abstract base class for all asynchronous operations.
     */
    class AsyncOperation {
    public:

        /**
         * Perform this operation.
         */
        virtual bool execute(KVStore *tut) {
            throw std::runtime_error("not implemented");
        }

    };

    /**
     * Operation sent to an async queue to tell it to shut down.
     */
    class AsyncShutdownOperation : public AsyncOperation {
    public:
        bool execute(KVStore *tut) {
            throw this;
        }
    };

    /**
     * Abstract base class for asynchronous operations with boolean
     * callbacks.
     */
    class BoolOperation : public AsyncOperation {
    public:

        /**
         * Store the callback.
         */
        BoolOperation(Callback<bool> *c) {
            cb = c;
        }

    protected:
        /**
         * The callback to be fired during execute().
         */
        Callback<bool> *cb;
    };

    /**
     * Async interface to reset.
     */
    class ResetOperation : public BoolOperation {
    public:
        ResetOperation(Callback<bool> *c) : BoolOperation(c) {}

        /**
         * Call reset, callback(true).
         */
        bool execute(KVStore *tut) {
            tut->reset();
            bool t = true;
            cb->callback(t);
            return true;
        }
    };

    /**
     * A NOOP that ensures it runs on the async's thread.
     */
    class NOOPOperation : public BoolOperation {
    public:
        NOOPOperation(Callback<bool> *c) : BoolOperation(c) {}

        /**
         * callback(true).
         */
        bool execute(KVStore *tut) {
            bool rv = true;
            cb->callback(rv);
            return true;
        }
    };

    /**
     * Async set operation.
     */
    class SetOperation : public BoolOperation {
    public:

        /**
         * Create a set with the given key, value and callback.
         */
        SetOperation(std::string &k, std::string &v,
                     Callback<bool> *c) : BoolOperation(c) {
            key = k;
            value = v;
        }

        /**
         * Call the underlying set method.
         */
        bool execute(KVStore *tut) {
            tut->set(key, value, *cb);
            return true;
        }
    private:
        std::string     key;
        std::string     value;
    };

    /**
     * Async delete operation.
     */
    class DeleteOperation : public BoolOperation {
    public:

        /**
         * Create a delete operation with the given key and callback.
         */
        DeleteOperation(std::string &k, Callback<bool> *c) : BoolOperation(c) {
            key = k;
        }

        /**
         * Execute the underlying delete.
         */
        bool execute(KVStore *db) {
            db->del(key, *cb);
            return true;
        }

    private:
        std::string key;
    };

    /**
     * Asynchronous get operation.
     */
    class GetOperation : public AsyncOperation {
    public:
        /**
         * Construct a GetOperation with the given key and callback.
         */
        GetOperation(std::string &k, Callback<GetValue> *c) {
            key = k;
            cb = c;
        }

        /**
         * Execute the underlying get and fire the result to the callback.
         */
        bool execute(KVStore *db) {
            db->get(key, *cb);
            return true;
        }

    private:
        std::string         key;
        Callback<GetValue> *cb;
    };

    /**
     * Async operations queue.
     */
    class AsyncQueue {
    public:

        /**
         * Create an async queue.
         */
        AsyncQueue() {
            pthread_mutex_init(&mutex, NULL);
            pthread_cond_init(&cond, NULL);
        }

        /**
         * Tear down an async queue.
         */
        ~AsyncQueue() {
            pthread_cond_destroy(&cond);
            pthread_mutex_destroy(&mutex);
        }

        /**
         * Add an operation to an async queue.
         *
         * @param op the operation to add
         */
        void addOperation(AsyncOperation *op) {
            LockHolder lh(&mutex);
            ops.push(op);
            if(pthread_cond_signal(&cond) != 0) {
                throw std::runtime_error("Error signaling change.");
            }
        }

        /**
         * Drain some operations to the given queue.
         *
         * This will remove as many operations from the async queue as
         * can be placed into the output queue given the maximum
         * execution number.
         *
         * @param out an output queue ready to receive the ops
         */
        void drainTo(std::queue<AsyncOperation*> &out) {
            LockHolder lh(&mutex);
            if(ops.empty()) {
                if(pthread_cond_wait(&cond, &mutex) != 0) {
                    throw std::runtime_error("Error waiting for signal.");
                }
            }
            for(int i = 0; i < MAX_DRAIN && !ops.empty(); i++) {
                out.push(ops.front());
                ops.pop();
            }
        }

    private:
        pthread_mutex_t             mutex;
        pthread_cond_t              cond;
        std::queue<AsyncOperation*> ops;
    };

    /**
     * Asynchronous executor.
     */
    class AsyncExecutor {
    public:

        /**
         * Construct an AsyncExecutor over the given underlying
         * KVStore with the given input queue.
         */
        AsyncExecutor(KVStore *d, AsyncQueue *q) {
            tut     = d;
            iq      = q;
        }

        /**
         * Run forever.
         */
        void run() {
            try {
                while(true) {
                    std::queue<AsyncOperation*> ops;
                    iq->drainTo(ops);
                    tut->begin();
                    int count = 0;

                    while(!ops.empty()) {
                        AsyncOperation *op = ops.front();
                        ops.pop();

                        op->execute(tut);
                        count++;
                        delete op;
                    }

                    tut->commit();
                }
            } catch(AsyncShutdownOperation *op) {
                std::cerr << "Shutting down..." << std::endl;
            } catch(std::runtime_error &e) {
                std::cerr << "Exception in executor loop: "
                          << e.what() << std::endl;
                abort();
            }
        }

        /**
         * Shut down this thread.
         */
        void stop() {
            iq->addOperation(new AsyncShutdownOperation());
        }

    private:
        KVStore       *tut;
        AsyncQueue    *iq;
    };

    static void* launch_executor_thread(void* arg) {
        AsyncExecutor *executor = (AsyncExecutor*) arg;
        try {
            executor->run();
        } catch(...) {
            std::cerr << "Caught a fatal exception in the thread" << std::endl;
        }
        return NULL;
    }

    /**
     * Asynchronous wrapper for a synchronous KVStore.
     */
    class QueuedKVStore : public KVStore {
    public:

        /**
         * Construct a QueuedKVStore wrapping the given thing.
         */
        QueuedKVStore(KVStore *t) {
            tut = t;
            iq = new AsyncQueue();
            executor = new AsyncExecutor(tut, iq);

            if(pthread_create(&thread, NULL, launch_executor_thread, executor)
               != 0) {
                throw std::runtime_error("Error initializing queue thread");
            }
        }

        /**
         * Clean up.
         */
        ~QueuedKVStore() {
            executor->stop();
            pthread_join(thread, NULL);
            delete executor;
            delete iq;
        }

        /**
         * Perform an async reset.
         */
        void reset() {
            std::queue<AsyncOperation*> opq;
            RememberingCallback<bool> cb;
            iq->addOperation(new ResetOperation(&cb));
            cb.waitForValue();
        }

        /**
         * Perform an async set.
         */
        void set(std::string &key, std::string &val,
                 Callback<bool> &cb) {
            iq->addOperation(new SetOperation(key, val, &cb));
        }

        /**
         * Perform an async get.
         */
        void get(std::string &key, Callback<GetValue> &cb) {
            iq->addOperation(new GetOperation(key, &cb));
        }

        /**
         * perform an async delete.
         */
        void del(std::string &key, Callback<bool> &cb) {
            iq->addOperation(new DeleteOperation(key, &cb));
        }

        /**
         * Perform a noop on the async thread (useful for verifying
         * everything is done).
         */
        void noop(Callback<bool> &cb) {
            iq->addOperation(new NOOPOperation(&cb));
        }

    private:
        KVStore *tut;
        AsyncQueue     *iq;
        AsyncExecutor  *executor;
        pthread_t       thread;
    };

}

#endif /* ASYNC_HH */
