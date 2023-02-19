#ifndef MY_CRON_H
#define MY_CRON_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#define TASKS_COUNT 50

typedef enum
{
    SET_TASK,
    CANCEL_TASK,
    LIST_TASKS
} MessageType;

typedef struct TaskRequest
{
    int is_absolute;
    struct itimerspec time;
} TaskRequest;

typedef struct Message
{
    MessageType type;
    union
    {
        TaskRequest request;
        int cancelled_task_id;
    };
} Message;

typedef struct Task
{
    int is_running;
    timer_t timer_id;
} Task;

struct Tasks
{
    int tasks_count;
    Task tasks[TASKS_COUNT];
};

int isServer();
void myCronInit(int argc, char *argv[]);

int myCronServer();
int myCronClient(int argc, char *argv[]);
timer_t create_timer();
int set_task(struct Tasks *tasks, int is_absolute, struct itimerspec value);
int cancel_task(struct Tasks *tasks, int task_id);
Message *parse_request(int argc, char *argv[]);
void *timer_notify(void *args);

#endif