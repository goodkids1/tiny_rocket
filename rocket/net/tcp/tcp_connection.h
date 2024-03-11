#ifndef ROCKET_NET_TCP_TCP_CONNECTION_H
#define ROCKET_NET_TCP_TCP_CONNECTION_H


#include"./net_addr.h"
#include"./tcp_buffer.h"
#include<memory>
#include<map>
#include<vector>
#include"../net/io_thread.h"
#include"../net/coder/abstract_coder.h"
#include"../net/rpc/rpc_dispatcher.h"

namespace rocket{
      enum TcpState{
            NotConnected = 1,
            Connected = 2,
            HalfClosing = 3,
            closed = 4,
        };

    enum TcpConnectionType{
        TcpConnectionByServer = 1,//作为服务端使用，代表跟对端客户端连接
        TcpConnectionByClient = 2,//作为客户端使用，代表跟对端服务端的连接
    };

    class TcpConnection{

        public:
        typedef std::shared_ptr<TcpConnection> s_ptr;
    
        //TcpConnection(IOThread* io_thread,int fd,int buffer_size,NetAddr::s_ptr peer_addr);
        TcpConnection(Eventloop* event_loop,int fd,int buffer_size,NetAddr::s_ptr peer_addr,TcpConnectionType type = TcpConnectionByServer);

        ~TcpConnection();

        void onread();

        void excute();

        void onwrite();

        void setState(const TcpState  state);

        TcpState getState();

        void clear();

        void shutdown();//服务器主打关闭连接

        void setConnectionType(TcpConnectionType type);

        void listenWrite();

        void listenRead();

        void pushSendMessage(AbstractProtocol::s_ptr messsage,std::function<void(AbstractProtocol::s_ptr)> done);
        void pushReadMessage(const std::string& req_id,std::function<void(AbstractProtocol::s_ptr)> done);
        private:
        Eventloop* m_event_loop {NULL};
        NetAddr::s_ptr m_local_addr;
        NetAddr::s_ptr m_peer_addr;

        TcpBuffer::s_ptr m_in_buffer;
        TcpBuffer::s_ptr m_out_buffer;

        FdEvent* m_fd_event{NULL};

        TcpState m_state;
        int m_fd{0};
        TcpConnectionType m_connection_type {TcpConnectionByServer};


        std::vector<std::pair<AbstractProtocol::s_ptr,std::function<void(AbstractProtocol::s_ptr)>>> m_write_dones;
        std::map<std::string,std::function<void(AbstractProtocol::s_ptr)>> m_read_dones;

        AbstractCoder* m_coder {NULL};

    }; 
}

#endif