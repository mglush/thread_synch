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
    lock->Acquire(); // lock
    while ((aw + ww) > 0) { // can only read if theres no writers (active or waiting)
        wr++; // if there are, we become a waiting reader.
        okToRead->Wait(lock); // wait for the read signal.
        wr--; // no longer an active reader.
    } // not safe to read while in this loop.
    ar++; // we are now an active reader.
    lock->Release(); // unlock
}

void RWLock::doneRead() {
    lock->Acquire(); // lock
    ar--; // no longer an active reader.
    if (ar == 0 && ww > 0) // no more active readers, wake up one writer.
        okToWrite->Signal(lock);
    lock->Release(); // unlock
}

void RWLock::startWrite() {
    lock->Acquire(); // lock
    while ((aw + ar) > 0) { // can only write when there's no active writers or readers.
        ww++; // we are a waiting writer.
        okToWrite->Wait(lock); // we wait for the signal to write.
        ww--; // no longer are a waiting writer.
    } // not safe to write while in this loop.
    aw++; // we are now an active writer.
    lock->Release(); // unlock
}

void RWLock::doneWrite() {
    lock->Acquire(); // lock
    aw--; // no longer an active writer.
    if (ww > 0) // wake up next writer.
        okToWrite->Signal(lock);
    else if (wr > 0) // wake up all readers.
        okToRead->Broadcast(lock);
    lock->Release(); // unlock
}
