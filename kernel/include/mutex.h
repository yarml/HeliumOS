#ifndef HELIUM_MUTEX_H
#define HELIUM_MUTEX_H

typedef volatile int mutex;

void mutex_lock(mutex *);
void mutex_ulock(mutex *);

#endif