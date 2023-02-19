#include "mycron.h"

const char *queue_name = "/cron_queuee";

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