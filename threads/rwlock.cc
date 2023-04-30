#include <stdio.h>
#include <pthread.h>
#include <iostream> 

#include "rwlock.h"

RWLock::RWLock() : ar(0), wr(0), aw(0), ww(0) {
    // initialize the mutex lock and the two condition variables.
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&okToRead, NULL);
    pthread_cond_init(&okToWrite, NULL);
}
RWLock::~RWLock() {
    // uninitialize the mutex lock and the two condition variables.
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&okToRead);
    pthread_cond_destroy(&okToWrite);
}

void RWLock::startRead() {
    pthread_mutex_lock(&lock);
    while ((aw + ww) > 0) {
        wr++;
        pthread_cond_wait(&okToRead, &lock);
        wr--;
    } // not safe to read.
    ar++; // we are now an active reader.
    pthread_mutex_unlock(&lock);
}

void RWLock::doneRead() {
    pthread_mutex_lock(&lock);
    ar--; // no longer an active reader.
    if (ar == 0 && ww > 0) // no more active readers, wake up one writer.
        pthread_cond_signal(&okToWrite);
    pthread_mutex_unlock(&lock);
}

void RWLock::startWrite() {
    pthread_mutex_lock(&lock);
    while ((aw + ar) > 0) {
        ww++;
        pthread_cond_wait(&okToWrite, &lock);
        ww--;
    } // not safe to write.
    aw++; // we are now an active writer.
    pthread_mutex_unlock(&lock);
}

void RWLock::doneWrite() {
    pthread_mutex_lock(&lock);
    aw--; // no longer an active writer.
    if (ww > 0) // wake up next writer.
        pthread_cond_signal(&okToWrite);
    else if (wr > 0) // wake up all readers.
        pthread_cond_broadcast(&okToRead);
    pthread_mutex_unlock(&lock);
}
