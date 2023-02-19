#import "mycron.h"

extern const char* queue_name;

int myCronServer()
{
    printf("my cron server\n");

    struct Tasks tasks;
    struct mq_attr attr;

    initialize_tasks(&tasks);

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(Message);

    mqd_t mq = mq_open(queue_name, O_RDONLY | O_CREAT, 0666, &attr);
    if (mq == -1)
    {
        printf("Failed to open server with errno %d\n", errno);
        return -1;
    }

    Message msg;

    while (1)
    {
        printf("Czekamy na wiadomosc\n");
        int ret = mq_receive(mq, (Message *)&msg, sizeof(Message), NULL);
        printf("otrzymalismy wiadomosc\n");
        switch (msg.type)
        {
        case SET_TASK:
            printf("%d", set_task(&tasks, 0, msg.request.time));
            break;
        case CANCEL_TASK:
            printf("cancel ret %d\n", cancel_task(&tasks, msg.cancelled_task_id));
            break;
        }
    }
}

timer_t create_timer()
{
    timer_t timer_id;
    struct sigevent timer_event;
    timer_event.sigev_notify = SIGEV_THREAD;
    timer_event.sigev_notify_function = timer_notify;
    timer_event.sigev_value.sival_ptr = NULL;
    timer_event.sigev_notify_attributes = NULL;
    timer_create(CLOCK_REALTIME, &timer_event, &timer_id);
    return timer_id;
}

int set_task(struct Tasks *tasks, int is_absolute, struct itimerspec value)
{
    for (int x = 0; x < tasks->tasks_count; x++)
    {

        if (tasks->tasks[x].is_running == 0)
        {
            timer_t timer_id = create_timer();

            timer_settime(timer_id, 0, &value, NULL);
            tasks->tasks[x].timer_id = timer_id;
            tasks->tasks[x].is_running = 1;
            printf("task id %d\n",x);
            return 0;
        }
    }

    return -1;
}

int cancel_task(struct Tasks *tasks, int task_id)
{
    if (!tasks)
        return NULL;

    if (tasks->tasks[task_id].is_running)
    {
        timer_delete(tasks->tasks[task_id].timer_id);
        tasks->tasks[task_id].is_running = 0;
        return 1;
    }

    return 0;
}

void initialize_tasks(struct Tasks *tasks)
{
    for (int x = 0; x < TASKS_COUNT; x++)
    {
        tasks->tasks[x].is_running = 0;
    }
}

void *timer_notify(void *args)
{
    printf("Odpalil sie handler elegancko\n");
}