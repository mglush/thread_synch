/**
 * Code is based on
 *http://www.algolist.net/Data_structures/Hash_table/Chaining
 * Use fine-grain rwlock or lock
 **/



#include <iostream>
#include <unistd.h>
//#include "rwlock.h"
#include "hashchain.h"
#include "synch.h"

//NOYIELD disables the yields used to help uncover synchronization bugs
//  - the yields are useful for uncovering bugs, but performance testing may be easier without them
#define NOYIELD 1

#ifndef NOYIELD
#ifdef NACHOS
#include "system.h" //for currentThread->Yield()
#define YIELD() currentThread->Yield()
#else //else pthreads
#define YIELD() pthread_yield()
#endif
#else //else NOYIELD
#define YIELD() do{}while(0)
#endif

//You don't need to use these, but if you use these macros then you only need to insert one line of code at each synchronization point down in the main code
//You can define the behavior for each lock call like follows:
//  #define START_READ() lock.lock()
//  #define END_READ() lock.unlock()
#ifdef NOLOCK
//no synchronization code
#define START_READ() do{}while(0)
#define END_READ() do{}while(0)
#define START_WRITE() do{}while(0)
#define END_WRITE() do{}while(0)
#elif defined P1_SEMAPHORE                      // semaphores have P() and V().
#define START_READ() sem[hash]->P()
#define END_READ() sem[hash]->V()
#define START_WRITE() sem[hash]->P()
#define END_WRITE() sem[hash]->V()
#elif defined P1_LOCK                           // locks have Acquire() and Release().
#define START_READ() lck[hash]->Acquire()
#define END_READ() lck[hash]->Release()
#define START_WRITE() lck[hash]->Acquire()
#define END_WRITE() lck[hash]->Release()
#elif defined P1_RWLOCK                         // rwlocks have startRead() doneRead() startWrite() doneWrite().
#define START_READ() rwlck[hash]->startRead()
#define END_READ() rwlck[hash]->doneRead()
#define START_WRITE() rwlck[hash]->startWrite()
#define END_WRITE() rwlck[hash]->doneWrite()
#else                                           //else behave like NOLOCK (no option passed)
#define START_READ() do{}while(0)
#define END_READ() do{}while(0)
#define START_WRITE() do{}while(0)
#define END_WRITE() do{}while(0)
#endif

LinkedHashEntry:: LinkedHashEntry(int key1, int value1) {
  this->key = key1;
  this->value = value1;
  this->next = NULL;
}

int 
LinkedHashEntry:: getKey() {
  return key;
}
int 
LinkedHashEntry:: getValue() {
  return value;
}

void 
LinkedHashEntry:: setValue(int value1) {
  this->value = value1;
}


LinkedHashEntry * 
LinkedHashEntry:: getNext() {
  return next;
}

void 
LinkedHashEntry:: setNext(LinkedHashEntry *next1) {
  this->next = next1;
}

const int TABLE_SIZE = 128;

HashMap::HashMap() {
  table = new LinkedHashEntry*[TABLE_SIZE];
  for (int i = 0; i < TABLE_SIZE; i++)
    table[i] = NULL;
#ifdef P1_SEMAPHORE // initialize each element of semaphore array with a new Semaphore.
  for (int i = 0; i < TABLE_SIZE; i++)
    sem[i] = new Semaphore(NULL, 1);
#elif defined P1_LOCK // initialize each element of lock array with a new Lock.
  for (int i = 0; i < TABLE_SIZE; i++)
    lck[i] = new Lock(NULL);
#elif defined P1_RWLOCK // initialize each element of rwlock array with a new RWLock.
  for (int i = 0; i < TABLE_SIZE; i++)
    rwlck[i] = new RWLock();
#endif
}

int 
HashMap::_get(int key) { //internal get() function. DO NOT MODIFY
  int hash = (key % TABLE_SIZE);
  if (table[hash] == NULL) {
    YIELD();
    return -1;
  } else {
    YIELD();
    LinkedHashEntry *entry = table[hash];
    while (entry != NULL && entry->getKey() != key) {
      entry = entry->getNext();
      YIELD();
    }
    if (entry == NULL) {
      YIELD();
      return -1;
    } else { 
      YIELD();
      return entry->getValue();
    }
  }
  return -1; //should never get here (just for complaining compilers)
}

void
HashMap::_put(int key, int value) { //internal put() function. DO NOT MODIFY
  int hash = (key % TABLE_SIZE);
  if (table[hash] == NULL) {
    YIELD();
    table[hash] = new LinkedHashEntry(key, value);
  } else {
    YIELD();
    LinkedHashEntry *entry = table[hash];
    while (entry->getNext() != NULL && entry->getKey() != key) {
      YIELD();
      entry = entry->getNext();
    }
    if (entry->getKey() == key) {
      YIELD();
      entry->setValue(value);
    } else {
      YIELD();
      entry->setNext(new LinkedHashEntry(key, value));
    }
  }
  YIELD();
}

int 
HashMap::get(int key1) { 
  int hash = (key1 % TABLE_SIZE); 
  START_READ();
  int ret= _get(key1);;
  END_READ();
  return ret;
}

void 
HashMap::put(int key1, int value1) { 
  int hash = (key1 % TABLE_SIZE); 
  START_WRITE();
  _put(key1,value1);
  END_WRITE();
}


void
HashMap::remove(int key) { 
  int hash = (key % TABLE_SIZE);
  START_WRITE();
  if (table[hash] != NULL) {
    YIELD();
    LinkedHashEntry *prevEntry = NULL;
    LinkedHashEntry *entry = table[hash];
    while (entry->getNext() != NULL && entry->getKey() != key) {
      YIELD();
      prevEntry = entry;
      entry = entry->getNext();
    }
    if (entry->getKey() == key) {
      YIELD();
      if (prevEntry == NULL) {
        LinkedHashEntry *nextEntry = entry->getNext();
        entry->setNext(NULL);
        delete entry;
        YIELD();
        table[hash] = nextEntry;
      } else {
        LinkedHashEntry *next = entry->getNext();
        entry->setNext(NULL);
        delete entry;
        YIELD();
        prevEntry->setNext(next);
      }
    }
  }
  END_WRITE();
}

HashMap:: ~HashMap() {
  for (int hash = 0; hash < TABLE_SIZE; hash++){
    if (table[hash] != NULL) {
      LinkedHashEntry *prevEntry = NULL;
      LinkedHashEntry *entry = table[hash];
      while (entry != NULL) {
        prevEntry = entry;
        entry = entry->getNext();
        delete prevEntry;
      }
    }
  }
  delete[] table;
}


void
HashMap::increment(int key, int value) { 
  int hash = (key % TABLE_SIZE); 
  START_WRITE();
  _put(key,_get(key)+value);
  END_WRITE();
}
