#ifndef __RWLOCK_H__
#define __RWLOCK_H__
#include<semaphore.h>
#include <pthread.h>

class RWLock{
private:
    unsigned int ar; // active readers.
    unsigned int wr; // waiting readers.
    unsigned int aw; // active writers.
    unsigned int ww; // waiting writers.

    pthread_mutex_t lock; // mutex lock.

    pthread_cond_t okToRead; // condition variable.
    pthread_cond_t okToWrite; // condition variable.
    
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
