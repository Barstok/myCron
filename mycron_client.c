#import "mycron.h"

extern const char *queue_name;

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

    sprintf(msg->response_queue, "%s_res_%d", queue_name, getpid());

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct Tasks);
    printf("%s\n", msg->response_queue);
    mqd_t mqr = mq_open((const char *)msg->response_queue, O_RDONLY | O_CREAT, 0666, &attr);
    if (mqr == -1)
    {
        printf("Failed to open response queue with errno %d\n", errno);
        return -1;
    }

    int ret = mq_send(mq, (const char *)msg, sizeof(Message), 1);

    void *response = malloc(sizeof(struct Tasks));

    ret = mq_receive(mqr, (char *)response, sizeof(struct Tasks), NULL);
    printf("ret %d %d\n", ret, errno);

    handle_response_from_service(msg->type, response);

    mq_close(mq);
    mq_close(mqr);
    mq_unlink(msg->response_queue);
    free(msg);
    free(response);
}

void handle_response_from_service(MessageType type, void *response)
{
    switch (type)
    {
    case SET_TASK:
        printf("Task has been set with id %d\n", ((struct Response *)response)->task_id);
        return;
    case CANCEL_TASK:
        printf("Task with id %d has been cancelled\n", ((struct Response *)response)->task_id);
        return;
    case LIST_TASKS:
        printf("Currently running tasks:\n");
        printf("Task ID\n");
        for (int x = 0; x < ((struct Tasks *)response)->tasks_count; x++)
        {
            printf("%d\n", ((struct Tasks *)response)->tasks[x].task_id);
        }
        break;
    }
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