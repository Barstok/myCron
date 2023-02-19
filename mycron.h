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
#include <string.h>

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
    char response_queue[50];
    union
    {
        TaskRequest request;
        int cancelled_task_id;
    };
} Message;

typedef struct Task
{
    int is_running;
    int task_id;
    timer_t timer_id;
} Task;

struct Tasks
{
    int tasks_count;
    Task tasks[TASKS_COUNT];
};

struct Response{
    int task_id;
};

int isServer();
void myCronInit(int argc, char *argv[]);

int myCronServer();
int myCronClient(int argc, char *argv[]);
timer_t create_timer();
int set_task(struct Tasks *tasks, int is_absolute, struct itimerspec value);
int cancel_task(struct Tasks *tasks, int task_id);
int get_all_running_tasks(struct Tasks *tasks_list, struct Tasks *tasks_table);
Message *parse_request(int argc, char *argv[]);
int respond_to_client(char* res_queue, MessageType res_type, int task_id, struct Tasks *tasks);
void handle_response_from_service(MessageType type, void *response);
void *timer_notify(void *args);

#endif