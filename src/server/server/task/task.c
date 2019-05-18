#include <string.h>
#include <stdlib.h>

#include "task.h"

void task_set_faulty(task_t* const task, const size_t buff_size, const void* const buff) {
    task->type = SEND_FAULTY;
    task->arg_size = buff_size;
    task->arg = malloc(buff_size);
    memcpy(task->arg, buff, buff_size);
}

void task_free(task_t* const task) {
    if(task->type == SEND_FAULTY)
        free(task->arg);
}
