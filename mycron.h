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

typedef struct Task
{
    int test;
} Task;

struct MyCron
{
    int tasks_count;
    Task tasks[50];
};

int isServer();
void myCronInit();

int myCronServer();
int myCronClient();
timer_t create_timer();
int set_task();
void* timer_notify(void* args);

#endif