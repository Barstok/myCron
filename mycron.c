#include "mycron.h"

static const char *queue_name = "/cron";

void myCronInit(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0); // wyrzuc to (tylko do debugowania)

    if (isServer())
        myCronServer();
    else
        myCronClient(argc, argv);
}

int isServer()
{
    mqd_t mq = mq_open(queue_name, O_WRONLY);

    if (mq == -1)
        return 1;
    printf("udalo sie otworzyc xddd\n");
    mq_close(mq);
    // mq_unlink(queue_name);
    return 0;
}

int myCronServer()
{
    printf("my cron server");

    struct Tasks tasks;
    struct mq_attr attr;
    tasks.tasks_count = TASKS_COUNT;
    // initialize_tasks(&tasks);

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

int myCronClient(int argc, char *argv[])
{
    printf("Client has started");
    mqd_t mq = mq_open(queue_name, O_WRONLY);
    printf("\n%s\n", queue_name);
    if (mq == -1)
    {
        printf("Failed to open client with errno %d\n", errno);
        return -1;
    }

    Message *msg = parse_request(argc, argv);

    int ret = mq_send(mq, (const char *)msg, sizeof(Message), 1);

    free(msg);
    mq_close(mq);
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

Message *parse_request(int argc, char *argv[])
{
    if (argc == 0)
        return NULL;

    Message *msg = malloc(sizeof(Message));

    if (strcmp("set", *(argv + 1)) == 0)
        msg->type = SET_TASK;
    else if (strcmp("cancel", *(argv + 1)) == 0)
        msg->type = CANCEL_TASK;
    else if (strcmp("list", *(argv + 1)) == 0)
        msg->type = LIST_TASKS;
    else
    {
        free(msg);
        return NULL;
    }

    switch (msg->type)
    {
    case SET_TASK:
        printf("set\n");

        if (strcmp(*(argv + 2), "-a") == 0)
        {
            printf("czas bezwzgledny");
        }

        int time = atoi(*(argv + 2));
        if (time < 0)
        {
            free(msg);
            return NULL;
        }
        int interval = 0;
        if (argc >= 4 && strcmp(*(argv + 3), "-i") == 0)
        {
            interval = atoi(*(argv + 4));
        }

        msg->request.time.it_value.tv_sec = time;
        msg->request.time.it_value.tv_nsec = 0;
        msg->request.time.it_interval.tv_sec = interval;
        msg->request.time.it_interval.tv_nsec = 0;

        return msg;
    case CANCEL_TASK:
        printf("cancel\n");
        msg->cancelled_task_id = atoi(*(argv + 2));
        return msg;
    case LIST_TASKS:
        printf("list\n");

        return msg;
    }
}

void initialize_tasks(struct Tasks *tasks)
{
    for (int x = 0; x < tasks->tasks_count; x++)
    {
        tasks->tasks[x].is_running = 0;
    }
}

void *timer_notify(void *args)
{
    printf("Odpalil sie handler elegancko\n");
}