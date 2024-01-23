#include <mutex.h>
#include <sys.h>

void mutex_lock(mutex *mutex) {
  while (!__sync_bool_compare_and_swap(mutex, 0, 1)) {
    pause();
  }
}

void mutex_ulock(mutex *mutex) {
  *mutex = 0;
}
