#import "mycron.h"

extern const char* queue_name;

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