#pragma once

#include <stdbool.h>
#include <pthread.h>

typedef struct rw_lock_struct
{
    int read_count;
    pthread_mutex_t mutex;
    pthread_mutex_t rw_mutex;
} rw_lock_t;

typedef rw_lock_t *rw_lock_ptr;

bool rw_create(rw_lock_ptr rw_lock);
bool rw_destroy(rw_lock_ptr rw_lock);

bool rw_acquire_reader(rw_lock_ptr rw_lock);
bool rw_release_reader(rw_lock_ptr rw_lock);

bool rw_acquire_writer(rw_lock_ptr rw_lock);
bool rw_release_writer(rw_lock_ptr rw_lock);
