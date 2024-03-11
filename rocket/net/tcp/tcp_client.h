#ifndef ROCKET_NET_TCP_TCP_CLIENT_H
#define ROCKET_NET_TCP_TCP_CLIENT_H
#include"./net_addr.h"
#include"./tcp_connection.h"
#include"../net/eventloop.h"
#include"../net/coder/abstract_protocol.h"


namespace rocket{

class TcpClient{
    public:
    TcpClient(NetAddr::s_ptr peer_addr);
    ~TcpClient();

    void connect(std::function<void()> done);//异步进行connect，如果connect成功，done会被执行

    void writeMessage(AbstractProtocol::s_ptr message,std::function<void(AbstractProtocol::s_ptr)> done);//异步发送Message 如果发送request成功 会调用done函数 函数的入参就是message对象

    void readMessage(const std::string& req_id,std::function<void(AbstractProtocol::s_ptr)> done);//异步读取Message 如果读取message成功 会调用done函数 函数的入参就是message对象



    private:
    NetAddr::s_ptr m_peer_addr;
    Eventloop* m_event_loop {NULL};
    int m_fd {-1};
    FdEvent* m_fd_event{NULL};
    TcpConnection::s_ptr m_connection;
};










}






#endif