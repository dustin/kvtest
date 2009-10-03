#ifndef LOCKS_H
#define LOCKS_H 1

#include <pthread.h>

#include "base-test.hh"

namespace kvtest {

    /**
     * pthread mutex holder (maintains lock while active).
     */
    class LockHolder {
    public:
        /**
         * Acquire the lock in the given mutex.
         */
        LockHolder(pthread_mutex_t *m) {
            mutex = m;
            if(pthread_mutex_lock(mutex) != 0) {
                throw std::runtime_error("Failed to acquire lock.");
            }
        }

        /**
         * Release the lock.
         */
        ~LockHolder() {
            pthread_mutex_unlock(mutex);
        }

    private:
        pthread_mutex_t *mutex;

        DISALLOW_COPY_AND_ASSIGN(LockHolder);
    };

}

#endif /* LOCKS_H */
