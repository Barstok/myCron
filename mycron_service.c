#import "mycron.h"

extern const char *queue_name;

static struct Tasks tasks;

static sem_t sem_tasks;

int myCronServer()
{
    printf("my cron server\n");

    struct mq_attr attr;

    sem_init(&sem_tasks, 0, 0);

    initialize_tasks(&tasks);

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(Message);

    mqd_t mq = mq_open(queue_name, O_RDONLY | O_CREAT, 0666, &attr);
    if (mq == -1)
    {
        return -1;
    }

    Message msg;
    struct Tasks task_list;
    while (1)
    {
        int ret = mq_receive(mq, (Message *)&msg, sizeof(Message), NULL);

        int id;

        switch (msg.type)
        {
        case SET_TASK:
            printf("%s", *msg.request.argv);
            id = set_task(&tasks, 0, msg.request.time, msg.request.task, msg.request.argv, msg.request.argc);
            break;
        case CANCEL_TASK:
            id = cancel_task(&tasks, msg.cancelled_task_id);
            break;
        case LIST_TASKS:
            get_all_running_tasks(&task_list, &tasks);
        }
        respond_to_client(msg.response_queue, msg.type, id, &task_list);
    }

    sem_destroy(&sem_tasks);
    return 0;
}

timer_t create_timer(Task *task)
{
    timer_t timer_id;
    struct sigevent timer_event;
    timer_event.sigev_notify = SIGEV_THREAD;
    timer_event.sigev_notify_function = timer_notify;
    timer_event.sigev_value.sival_ptr = task;
    timer_event.sigev_notify_attributes = NULL;
    timer_create(CLOCK_REALTIME, &timer_event, &timer_id);
    return timer_id;
}

int set_task(struct Tasks *tasks, int is_absolute, struct itimerspec value, char *task, char **argv, int argc)
{
    for (int x = 0; x < TASKS_COUNT; x++)
    {
        if (tasks->tasks[x].is_running == 0)
        {
            tasks->tasks[x].task = task;
            printf("ARGC = %d\n", argc);
            tasks->tasks[x].argv = parse_argv(argv, argc);
            timer_t timer_id = create_timer(&tasks->tasks[x]);

            timer_settime(timer_id, 0, &value, NULL);
            tasks->tasks[x].timer_id = timer_id;
            tasks->tasks[x].is_running = 1;
            if (value.it_interval.tv_sec)
                tasks->tasks[x].is_cyclic = 1;
            else
                tasks->tasks[x].is_cyclic = 0;
            tasks->tasks[x].task_id = x;
            return x;
        }
    }

    return -1;
}

int cancel_task(struct Tasks *tasks, int task_id)
{
    if (!tasks)
        return -1;
    if (tasks->tasks[task_id].is_running)
    {
        timer_delete(tasks->tasks[task_id].timer_id);
        tasks->tasks[task_id].is_running = 0;
        return task_id;
    }

    return -1;
}

void initialize_tasks(struct Tasks *tasks)
{
    sem_wait(&sem_tasks);
    for (int x = 0; x < TASKS_COUNT; x++)
    {
        tasks->tasks[x].is_running = 0;
    }
    sem_post(&sem_tasks);
}

int get_all_running_tasks(struct Tasks *tasks_list, struct Tasks *tasks_table)
{
    tasks_list->tasks_count = 0;
    sem_wait(&sem_tasks);
    for (int x = 0; x < TASKS_COUNT; x++)
    {
        if (tasks_table->tasks[x].is_running == 1)
        {
            memcpy(&tasks_list->tasks[tasks_list->tasks_count], &tasks_table->tasks[x], sizeof(Task));
            tasks_list->tasks_count++;
        }
    }
    sem_wait(&sem_tasks);
}

int respond_to_client(char *res_queue, MessageType res_type, int task_id, struct Tasks *tasks)
{
    if (!res_queue)
        return NULL;

    int mq;
    mq = mq_open(res_queue, O_WRONLY);
    if (mq == -1)
        return -1;

    struct Response res = {.task_id = task_id};

    switch (res_type)
    {
    case SET_TASK:
    case CANCEL_TASK:
        mq_send(mq, (const char *)&res, sizeof(struct Response), 0);
        break;
    case LIST_TASKS:
        mq_send(mq, (const char *)tasks, sizeof(struct Tasks), 0);
        break;
    }
    mq_close(mq);
    return 0;
}

void *timer_notify(void *args)
{
    if (!args)
        return NULL;
    printf("Odpalil sie handler elegancko\n");
    sem_wait(&sem_tasks);
    struct Task *task = (struct Task *)args;
    printf("timer notify task_id %d\n", task->task_id);

    pid_t pid;
    printf("posix spawn\n");
    printf("%s", task->task);
    char *argv[] = {"test", "xd", "pdw", NULL};
    int status = posix_spawn(&pid, task->task, NULL, NULL, argv, NULL);
    if(!task->is_cyclic){
        timer_delete(task->timer_id);
        task->is_running = 0;
    }
    sem_wait(&sem_tasks);
    printf("Status %d\n", status);
}

char** parse_argv(char** argv, int count){
    char **p_argv = calloc(10, sizeof(char*));

    for(int x=0; x<count;x++){
        *(p_argv+x) = *(argv+x);
    }
    
    return p_argv;
}