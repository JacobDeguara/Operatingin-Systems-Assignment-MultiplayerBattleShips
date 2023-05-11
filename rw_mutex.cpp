#include "rw_mutex.h"

#include <stdlib.h>

#define CATASTROPHIC_EXIT_CODE -1

bool rw_create(rw_lock_ptr rw_lock)
{
    rw_lock->read_count = 0;
    if (pthread_mutex_init(&rw_lock->mutex, NULL) != 0)
        return false;
    if (pthread_mutex_init(&rw_lock->rw_mutex, NULL) != 0)
    {
        pthread_mutex_destroy(&rw_lock->mutex);
        return false;
    }

    return true;
}

bool rw_destroy(rw_lock_ptr rw_lock)
{
    rw_lock->read_count = 0;
    bool retVal = pthread_mutex_destroy(&rw_lock->mutex) == 0;
    retVal &= pthread_mutex_destroy(&rw_lock->rw_mutex) == 0;
    return retVal;
}

bool rw_acquire_reader(rw_lock_ptr rw_lock)
{
    // Increment read count
    if (pthread_mutex_lock(&rw_lock->mutex) != 0)
        return false;

    // Lock the rw_lock to block writers out
    if (++rw_lock->read_count == 1)
    {
        if (pthread_mutex_lock(&rw_lock->rw_mutex) != 0)
        {
            --rw_lock->read_count;
            if (pthread_mutex_unlock(&rw_lock->mutex) != 0)
                exit(CATASTROPHIC_EXIT_CODE);
            return false;
        }
    }

    if (pthread_mutex_unlock(&rw_lock->mutex) != 0)
        exit(CATASTROPHIC_EXIT_CODE);

    return true;
}

bool rw_release_reader(rw_lock_ptr rw_lock)
{
    if (pthread_mutex_lock(&rw_lock->mutex) != 0)
        return false;

    // Unlock the rw_lock to let writers in
    if (--rw_lock->read_count == 0)
    {
        if (pthread_mutex_unlock(&rw_lock->rw_mutex) != 0)
        {
            ++rw_lock->read_count;
            if (pthread_mutex_unlock(&rw_lock->mutex) != 0)
                exit(CATASTROPHIC_EXIT_CODE);
            return false;
        }
    }

    if (pthread_mutex_unlock(&rw_lock->mutex) != 0)
        exit(CATASTROPHIC_EXIT_CODE);

    return true;
}

bool rw_acquire_writer(rw_lock_ptr rw_lock)
{
    return pthread_mutex_lock(&rw_lock->rw_mutex) == 0;
}

bool rw_release_writer(rw_lock_ptr rw_lock)
{
    return pthread_mutex_unlock(&rw_lock->rw_mutex) == 0;
}
