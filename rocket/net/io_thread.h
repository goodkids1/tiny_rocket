#ifndef ROCKET_NET_IO_THREAD_H
#define ROCKET_NET_IO_THREAD_H

#include"./eventloop.h"
#include<pthread.h>
#include<semaphore.h>

namespace rocket{
class IOThread{

public:
 IOThread();

 ~IOThread();

 Eventloop* getEventLoop();

 static void* Main(void* arg);
 void start();
 void join();

 private:
 pid_t m_thread_id{-1};
 pthread_t m_thread{0};

 Eventloop* m_event_loop{NULL};
 sem_t m_init_semaphore;
 sem_t m_start_semaphore;

};
}
#endif