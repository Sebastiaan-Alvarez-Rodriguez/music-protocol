#ifndef TASK
#define TASK

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


#endif
