#ifndef __RWLOCK_H__
#define __RWLOCK_H__

#include "synch.h"

class RWLock{
private:
    unsigned int ar; // active readers.
    unsigned int wr; // waiting readers.
    unsigned int aw; // active writers.
    unsigned int ww; // waiting writers.

    Lock *lock; // mutex lock using class we just wrote.

    Condition *okToRead; // condition variable using class we just wrote.
    Condition *okToWrite; // condition variable using class we just wrote.
    
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
