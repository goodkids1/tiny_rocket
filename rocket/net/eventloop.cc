#include"./eventloop.h"
#include "../common/log.h"
#include "../common/util.h"
#include<sys/epoll.h>
#include<sys/socket.h>
#include<sys/eventfd.h>
#include<string.h>

#define ADD_TO_EPOLL()                               \
    auto it = m_listen_fds.find(fdevent->getfd());\
    int op = EPOLL_CTL_ADD;                        \
    if(it != m_listen_fds.end())                   \
    {                                              \
        op = EPOLL_CTL_MOD;                        \
    }                                              \
    epoll_event tmp = fdevent->getEpollEvent();    \
    int rt = epoll_ctl(m_epoll_fd,op,fdevent->getfd(),&tmp);\
    if(rt == -1)                                              \
    {                                                         \   
        DEBUGLOG("调整后op = [%d]",op);     \          
        ERRORLOG("failed epoll_ctl when add fd %d,errno = %d, error = %s",fdevent->getfd(),errno,strerror(errno));\
    } \
    m_listen_fds.insert(fdevent->getfd());                                                                                                           \
    DEBUGLOG("add event success,fd[%d]",fdevent->getfd());                            \

#define DELETE_TO_EPOLL()\
    auto it = m_listen_fds.find(fdevent->getfd());\
    if(it == m_listen_fds.end())\
    {\
        return;\
    }\
    int op = EPOLL_CTL_DEL;\
    epoll_event tmp = fdevent->getEpollEvent();\
    int rt = epoll_ctl(m_epoll_fd,op,fdevent->getfd(),&tmp);\
    if(rt == -1)\
    {\
        ERRORLOG("failed epoll_ctl when delete fd %d",fdevent->getfd());\
    }\
    m_listen_fds.erase(fdevent->getfd()); \
    DEBUGLOG("delete event success,fd[%d]",fdevent->getfd());    \

namespace rocket
{
    static thread_local Eventloop* t_current_eventloop=NULL;
    static int g_epoll_max_timeout = 1000;
    static int g_epoll_max_events = 10;

    Eventloop::Eventloop()
    {
        if(t_current_eventloop!=NULL){
            ERRORLOG("failed to create event loop,this thread has created event loop");
            exit(0);
        }
        m_thread_id = getThreadId();
        m_epoll_fd = epoll_create(1);
        if(m_epoll_fd==-1)
        {
            ERRORLOG("failed to create event loop,epoll create error,error info[%d]",errno);
            exit(0);
        } 
        initWakeUpFdEvent();
        initTimer();
        INFOLOG("create event loop in thread %d",m_thread_id);
        t_current_eventloop = this;

    }
    Eventloop::~Eventloop()
    {
        close(m_epoll_fd);
        if(m_wakeup_fd_event){
            delete m_wakeup_fd_event;
            m_wakeup_fd_event = NULL;
        }
        if(m_timer){
            delete m_timer;
            m_timer = NULL;
        }
    }
    void Eventloop::initTimer()
    {
        m_timer = new Timer();
        addEpollEvent(m_timer);

    }
    void Eventloop::addTimerEvent(TimerEvent::s_ptr event)
    {
        m_timer->addTimerEvent(event);
    }
    void Eventloop::initWakeUpFdEvent()
    {
        m_wakeup_fd = eventfd(0,EFD_NONBLOCK);
        if(m_wakeup_fd<=0)
        {
            ERRORLOG("failed to create event loop,eventfd create error,error info[%d]",errno);
            exit(0);
        }
        m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);
        m_wakeup_fd_event->listen(FdEvent::IN_EVENT,[this](){
            char buf[8];
            while(read(m_wakeup_fd,buf,8)!=-1&&errno!=EAGAIN){
            }
            DEBUGLOG("read full bytes from wakeup fd[%d]",m_wakeup_fd);});

        addEpollEvent(m_wakeup_fd_event);

    }
    void Eventloop::loop()
    {
        m_is_looping = true;
        while(!m_stop_flag)
        {
            ScopeMutext<Mutex> lock(m_mutex);
            std::queue<std::function<void()>> tmp_tasks;
            m_pending_tasks.swap(tmp_tasks);
            lock.unlock();
            while(!tmp_tasks.empty())
            {
                std::function<void()> cb = tmp_tasks.front();
                tmp_tasks.pop();
                if(cb){
                    cb();
                }
            }
            //如果有定时任务需要执行 在这里执行
            //怎么判断有定时任务需要执行？ （TimerEvent.arrive_time)
            int timeout = g_epoll_max_timeout;
            epoll_event result_events[g_epoll_max_events];
           // DEBUGLOG("now begin to epoll_wait");
            int rt = epoll_wait(m_epoll_fd,result_events,g_epoll_max_events,timeout);
          //  DEBUGLOG("now begin to epoll_wait,rt = %d",rt);
            if(rt<0)
            {
                ERRORLOG("epoll_wait error,error info[%d]",errno);

            }
            else{
                for(int i=0;i<rt;i++)
                {
                    epoll_event trigger_event = result_events[i];
                    FdEvent* fdevent = static_cast<FdEvent*> (trigger_event.data.ptr);
                    if(fdevent == NULL)
                    {
                        continue;
                    }
                    if(trigger_event.events & EPOLLIN)
                    {
                        DEBUGLOG("fd %d trigger EPOLLIN event",fdevent->getfd());
                        addtask(fdevent->handler(FdEvent::IN_EVENT));
                    }
                    if(trigger_event.events & EPOLLOUT)
                    {
                        DEBUGLOG("fd %d trigger EPOLLOUT event",fdevent->getfd());
                        addtask(fdevent->handler(FdEvent::OUT_EVENT));
                    }
                }
            }
        }

    }
    void Eventloop::wakeup()
    {
        m_wakeup_fd_event->wakeup();
    }
    void Eventloop::addEpollEvent(FdEvent* fdevent)
    {
        if(isInLoopThread())
        {
          ADD_TO_EPOLL();
        }
            else{
                auto cb = [this,fdevent](){
                    ADD_TO_EPOLL();
                };
                addtask(cb,true);
            }
    }

    void Eventloop::deleteEpollEvent(FdEvent* fdevent)
    {
      if(isInLoopThread())
        {
            DELETE_TO_EPOLL(); 
        }
        else{
        auto cb = [this,fdevent](){
           DELETE_TO_EPOLL();
        };
        addtask(cb,true);
        }
    }
    bool Eventloop::isInLoopThread()
    {
        return getThreadId()==m_thread_id;
    }
    void Eventloop::addtask(std::function<void()> cb,bool is_wake_up)
    {
        ScopeMutext<Mutex> lock(m_mutex);
        m_pending_tasks.push(cb);
        lock.unlock();
        if(is_wake_up)
        {
            wakeup();
        }
    }
    void Eventloop::stop()
    {
        m_stop_flag = true;
    }

    Eventloop* Eventloop::GetCurrentEventLoop()
    {
        if(t_current_eventloop){
            return t_current_eventloop;
        }
        t_current_eventloop = new Eventloop();
        return t_current_eventloop;

    }
    bool Eventloop::islooping(){
        return m_is_looping;
    }
    
    void Eventloop::dealwakeup()
    {

    }

}


