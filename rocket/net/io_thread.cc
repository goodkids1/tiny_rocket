#include"./io_thread.h"
#include<assert.h>
#include "../common/log.h"
#include "../common/util.h"
#include<pthread.h>

namespace rocket{
    IOThread::IOThread()
    {
        int rt = sem_init(&m_init_semaphore,0,0);
        sem_init(&m_start_semaphore,0,0);
        assert(rt==0);

        pthread_create(&m_thread,NULL,&IOThread::Main,this);
        sem_wait(&m_init_semaphore);
        DEBUGLOG("IOThread %d create success",m_thread_id);
    }
    IOThread::~IOThread()
    {
        m_event_loop->stop();
        sem_destroy(&m_init_semaphore);
        sem_destroy(&m_start_semaphore);
        pthread_join(m_thread,NULL);
        if(m_event_loop!=NULL){
            delete m_event_loop;
            m_event_loop = NULL;
        }
    }

    void* IOThread::Main(void* arg)
    {
        IOThread* thread = static_cast<IOThread*> (arg);
        thread->m_event_loop = new Eventloop();
        thread->m_thread_id = getThreadId();
        sem_post(&thread->m_init_semaphore);
        DEBUGLOG("IOThread %d create,wait start semaphore",thread->m_thread_id);
        sem_wait(&thread->m_start_semaphore);
        DEBUGLOG("IOThread %d start loop",thread->m_thread_id);
        thread->m_event_loop->loop();
        DEBUGLOG("IOThread %d end loop",thread->m_thread_id);
        return NULL;
    }

    Eventloop* IOThread::getEventLoop()
    {
        return m_event_loop;
    }

    void IOThread::start()
    {
        DEBUGLOG("Now invoke IOThread %d",m_thread_id);
        sem_post(&m_start_semaphore);
    }

    void IOThread::join()
    {
        pthread_join(m_thread,NULL);
    }
}