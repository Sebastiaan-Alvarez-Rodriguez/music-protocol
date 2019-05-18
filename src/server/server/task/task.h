#ifndef TASK
#define TASK

#include <stddef.h>

typedef enum {
    SEND_BATCH,
    SEND_FAULTY,
    SEND_EOS,
    SEND_ACK
} task_type;

typedef struct {
    task_type type;
    size_t arg_size;
    void* arg;
} task_t;

void task_set_faulty(task_t* const task, const size_t buff_size, const void* const buff);

void task_free(task_t* const task);

#endif
