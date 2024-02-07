#ifndef HELIUM_TASK_H
#define HELIUM_TASK_H

#include <stddef.h>

typedef enum TASK_STATE {
  TASK_BLOCKING,
  TASK_RUNNING,
  TASK_PENDING,
  TASK_ZOMBIE
} task_state;

typedef struct TASK {
  size_t     id;
  task_state state;
} task;

#endif