#ifndef CALLBACKS_H
#define CALLBACKS_H 1

namespace kvtest {

    /**
     * Interface for callbacks from storage APIs.
     */
    template <typename RV>
    class Callback {
    public:
        /**
         * Method called on callback.
         */
        virtual void callback(RV &value) {
            throw std::runtime_error("Nobody should call this.");
        }
    };

    template <typename T>
    class RememberingCallback : public Callback<T> {
    public:
        RememberingCallback() {
            if(pthread_mutex_init(&mutex, NULL) != 0) {
                throw std::runtime_error("Failed to initialize mutex.");
            }
            if(pthread_cond_init(&cond, NULL) != 0) {
                throw std::runtime_error("Failed to initialize condition.");
            }
            fired = false;
        }

        RememberingCallback(RememberingCallback &copy) {
            throw std::runtime_error("Copying!");
        }

        ~RememberingCallback() {
            pthread_mutex_destroy(&mutex);
            pthread_cond_destroy(&cond);
        }

        void callback(T &value) {
            LockHolder lh(&mutex);
            val = value;
            fired = true;
            if(pthread_cond_broadcast(&cond) != 0) {
                throw std::runtime_error("Failed to broadcast change.");
            }
        }

        void waitForValue() {
            LockHolder lh(&mutex);
            if (!fired) {
                if(pthread_cond_wait(&cond, &mutex) != 0) {
                    throw std::runtime_error("Failed to wait for condition.");
                }
            }
            assert(fired);
        }

        T               val;
        bool            fired;

    private:
        pthread_mutex_t mutex;
        pthread_cond_t  cond;
    };

}

#endif /* CALLBACKS_H */
