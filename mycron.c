#include "mycron.h"

static const char* queue_name = "/cron";

void myCronInit(){
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("mycron init\n");
    if(isServer()) myCronServer();
    else myCronClient();
}

int isServer(){
    mqd_t mq = mq_open(queue_name, O_WRONLY);

    if(mq == -1) return 1;
    printf("udalo sie otworzyc xddd\n");
    mq_close(mq);
    //mq_unlink(queue_name);
    return 0;
}

int myCronServer(){
    printf("my cron server");
    
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 10;

    mqd_t mq = mq_open(queue_name, O_RDONLY | O_CREAT, 0666, &attr);
    if(mq == -1){
        printf("Failed to open server with errno %d\n",errno);
        return -1;
    }
    char msg[100];

    while (1){
        printf("Czekamy na wiadomosc\n");
        int ret = mq_receive(mq, msg, 100, NULL);
        set_task();
        printf("%s\n", msg);
        sleep(5);
    }

}

int myCronClient(){
    printf("Client has started");
    mqd_t mq = mq_open(queue_name, O_WRONLY);
    printf("\n%s\n", queue_name);
    if(mq == -1){
        printf("Failed to open client with errno %d\n",errno);
        return -1;
    }

    char* msg = "test";

    int ret = mq_send(mq,(const char*) msg, strlen(msg)+1, 1);
    printf("%d", ret);

    mq_close(mq);

    mq_unlink(queue_name);
}

timer_t create_timer(){
    timer_t timer_id;
    struct sigevent timer_event;
    timer_event.sigev_notify = SIGEV_THREAD;
    timer_event.sigev_notify_function = timer_notify;
    timer_event.sigev_value.sival_ptr = NULL;
	timer_event.sigev_notify_attributes = NULL;
    timer_create(CLOCK_REALTIME, &timer_event, &timer_id);

    return timer_id;
}

int set_task(){
    timer_t timer_id = create_timer();

    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);

    struct itimerspec value;
    value.it_value.tv_sec = current_time.tv_sec + 5;
    value.it_value.tv_nsec = current_time.tv_nsec + 0;
    value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = 0;

    timer_settime(timer_id, TIMER_ABSTIME, &value, NULL);

    return 0;
}

void* timer_notify(void* args){
    printf("Odpalil sie handler elegancko\n");
}