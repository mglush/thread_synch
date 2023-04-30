#include <stdio.h>
#include <iostream> 

#include "rwlock.h"

RWLock::RWLock() : ar(0), wr(0), aw(0), ww(0) {
    // initialize the mutex lock and the two condition variables.
    lock = new Lock(NULL);
    okToRead = new Condition(NULL);
    okToWrite = new Condition(NULL);
}
RWLock::~RWLock() {
    // uninitialize the mutex lock and the two condition variables.
    delete lock;
    delete okToRead;
    delete okToWrite;
}

void RWLock::startRead() {
    lock->Acquire();
    while ((aw + ww) > 0) {
        wr++;
        okToRead->Wait(lock);
        wr--;
    } // not safe to read.
    ar++; // we are now an active reader.
    lock->Release();
}

void RWLock::doneRead() {
    lock->Acquire();
    ar--; // no longer an active reader.
    if (ar == 0 && ww > 0) // no more active readers, wake up one writer.
        okToWrite->Signal(lock);
    lock->Release();
}

void RWLock::startWrite() {
    lock->Acquire();
    while ((aw + ar) > 0) {
        ww++;
        okToWrite->Wait(lock);
        ww--;
    } // not safe to write.
    aw++; // we are now an active writer.
    lock->Release();
}

void RWLock::doneWrite() {
    lock->Acquire();
    aw--; // no longer an active writer.
    if (ww > 0) // wake up next writer.
        okToWrite->Signal(lock);
    else if (wr > 0) // wake up all readers.
        okToRead->Broadcast(lock);
    lock->Release();
}
