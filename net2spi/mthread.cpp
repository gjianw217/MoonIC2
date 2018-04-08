#include "mthread.h"

void* MThread::run0(void* pVoid)
{
    MThread* p = (MThread*) pVoid;
    p->run1();
    return p;
}

void* MThread::run1()
{

    threadStatus = THREAD_STATUS_RUNNING;
    tid = pthread_self();
    run();
    threadStatus = THREAD_STATUS_EXIT;
    tid = 0;
    pthread_exit(NULL);
}

MThread::MThread()
{
    tid = 0;
    threadStatus = THREAD_STATUS_NEW;
}

bool MThread::start()
{
    return pthread_create(&tid, NULL, run0, this) == 0;
}

pthread_t MThread::getMThreadID()
{
    return tid;
}

int MThread::getState()
{
    return threadStatus;
}

void MThread::join()
{
    if (tid > 0)
    {
        pthread_join(tid, &tret);
    }
}

void MThread::join(unsigned long millisTime)
{

    if (tid == 0)
    {
        return;
    }
    if (millisTime == 0)
    {
        join();
    }else
    {
        unsigned long k = 0;
        while (threadStatus != THREAD_STATUS_EXIT && k <= millisTime)
        {
            usleep(100);
            k++;
        }
    }
}