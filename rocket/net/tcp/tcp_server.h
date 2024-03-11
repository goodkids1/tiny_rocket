#ifndef ROCKET_NET_TCP_SERVER_H
#define ROCKET_NET_TCP_SERVER_H
#include"./tcp_acceptor.h"
#include"./net_addr.h"
#include<set>
#include"./tcp_connection.h"
#include"../net/eventloop.h"
#include"../net/io_thread_group.h"

namespace rocket{

    class TcpServer{
        public:
        TcpServer(NetAddr::s_ptr local_addr);

        ~TcpServer();

        void start();

        void init();

        void onAccept();//当有新客户端连接之后需执行

        private:
        TcpAcceptor::s_ptr m_acceptor;
        NetAddr:: s_ptr m_local_addr;//本地监听套接字

        Eventloop* m_main_event_loop{NULL};//mainReactor
        IOThreadGroup* m_io_thread_group{NULL};//subReactor组
        FdEvent* m_listen_fd_event;
        int m_client_counts{0};
        std::set<TcpConnection::s_ptr> m_client;
    };

}
#endif