#ifndef ROCKET_NET_EVENTLOOP_H
#define ROCKET_NET_EVENTLOOP_H

#include<pthread.h>
#include<set>
#include<functional>
#include<queue>
#include "/home/zxl/rocket/rocket/common/mutex.h"
#include"./fd_event.h"
#include"./wakeup_fd_event.h"
#include"./timer.h"

namespace rocket
{
    class Eventloop
    {

        public:
        Eventloop();
        ~Eventloop();

        void loop();
        void wakeup();
        void stop();
        void addEpollEvent(FdEvent* fdevent);
        void deleteEpollEvent(FdEvent* fdevent);
        bool isInLoopThread();
        void addTimerEvent(TimerEvent::s_ptr event);
        static Eventloop* GetCurrentEventLoop();
        bool islooping();

        private:
        void dealwakeup();
        void addfd();
        void addtask(std::function<void()> cb,bool is_wake_up = false);
        void initWakeUpFdEvent();
        void initTimer();
        bool m_is_looping {false};

        private:
        pid_t m_thread_id;
        std::set<int> m_listen_fds;
        int m_epoll_fd {0};
        bool m_stop_flag {false};
        int m_wakeup_fd {0};
        std::queue<std::function<void()>> m_pending_tasks;
        Mutex m_mutex;
        WakeUpFdEvent* m_wakeup_fd_event {NULL};
        Timer* m_timer {NULL};
    };
}


#endif