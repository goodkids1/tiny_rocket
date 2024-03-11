#include"./tcp_client.h"
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<memory>
#include"/home/zxl/rocket/rocket/common/log.h"
#include"/home/zxl/rocket/rocket/net/fd_event_group.h"

namespace rocket
{
    TcpClient::TcpClient(NetAddr::s_ptr peer_addr):m_peer_addr(peer_addr){
        m_event_loop = Eventloop::GetCurrentEventLoop();
        m_fd = socket(peer_addr->GetFamily(),SOCK_STREAM,0);
        if(m_fd<0){
            ERRORLOG("tcpclient::tcpclient() error,failed to create fd");
            return;
        }
        m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);
        m_fd_event->setNonBlock();
        m_connection = std::make_shared<TcpConnection>(m_event_loop,m_fd,128,peer_addr,TcpConnectionByClient);
        m_connection->setConnectionType(TcpConnectionByClient);
    }

    TcpClient::~TcpClient(){
        DEBUGLOG("~TcpClient()");
        if(m_fd >0){
            close(m_fd);
        }
    }

    void TcpClient::connect(std::function<void()> done){
        int rt =::connect(m_fd,m_peer_addr->getSockAddr(),m_peer_addr->getSocklen());
        if(rt==0){
            DEBUGLOG("connect success");
            if(done){
                done();
            }
        }else if(rt==-1){
            if(errno == EINPROGRESS){
                m_fd_event->listen(FdEvent::OUT_EVENT,[this,done](){
                    int error = 0;
                    socklen_t len = sizeof(error);
                    getsockopt(m_fd,SOL_SOCKET,SO_ERROR,&error,&len);
                    bool is_connect_succ = false;
                    if(error == 0){
                        DEBUGLOG("connect [%s] success",m_peer_addr->toString().c_str());
                       is_connect_succ = true;
                       m_connection->setState(Connected);
                    }else{
                        ERRORLOG("connect error,errno = %d,error = %s",errno,strerror(errno));
                    }
                    m_fd_event->cancel(FdEvent::OUT_EVENT);
                    m_event_loop->addEpollEvent(m_fd_event);
                    if(is_connect_succ && done){
                        done();
                    }
                });
                m_event_loop->addEpollEvent(m_fd_event);
                if(!m_event_loop->islooping()){
                    m_event_loop->loop();
                }
              
            }else{
                ERRORLOG("connect error,errno = %d,error = %s",errno,strerror(errno));
            }
        }
        //  if(!m_event_loop->islooping()){
        //     m_event_loop->loop();
        // }
    }

    //异步发送message 如果发送成功会调用done函数
    void TcpClient::writeMessage(AbstractProtocol::s_ptr message,std::function<void(AbstractProtocol::s_ptr)> done){
        //把message对象写入到connection的buffer，done也写入  启动connection可写事件
        m_connection->pushSendMessage(message,done);
        m_connection->listenWrite();
    }

     //异步读取message 如果读取成功会调用done函数
    void TcpClient::readMessage(const std::string& req_id,std::function<void(AbstractProtocol::s_ptr)> done){
        //1.监听可读事件
        //2.从buffer里decode得到message对象，判断req_id是否相等，相等则读成功，执行其回调
        m_connection->pushReadMessage(req_id,done);
        m_connection->listenRead();

    }
}