#ifndef __RWLOCK_H__
#define __RWLOCK_H__

#include "synch.h"

class RWLock{
private:
    unsigned int ar; // active readers.
    unsigned int wr; // waiting readers.
    unsigned int aw; // active writers.
    unsigned int ww; // waiting writers.

    Lock *lock; // mutex lock.

    Condition *okToRead; // condition variable.
    Condition *okToWrite; // condition variable.
    
public:
    RWLock();
    ~RWLock();
    
    // reader.
    void startRead();
    void doneRead();
    
    // writer.
    void startWrite();
    void  doneWrite();
};

#endif
