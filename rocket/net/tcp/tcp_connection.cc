#include"./tcp_connection.h"
#include"/home/zxl/rocket/rocket/common/log.h"
#include"/home/zxl/rocket/rocket/net/fd_event_group.h"
#include<unistd.h>
#include"/home/zxl/rocket/rocket/net/coder/tinypb_coder.h"

namespace rocket{

    TcpConnection::TcpConnection(Eventloop* event_loop,int fd,int buffer_size,NetAddr::s_ptr peer_addr,TcpConnectionType type):m_event_loop(event_loop),m_peer_addr(peer_addr),m_state(NotConnected),m_fd(fd),m_connection_type(type)
    {
        m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
        m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);
        m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
        m_fd_event->setNonBlock();
        m_coder = new TinyPBCoder();
        if(m_connection_type == TcpConnectionByServer){
            listenRead();
        }

    }

    TcpConnection::~TcpConnection(){
        DEBUGLOG("~TcpConnection");
        if(m_coder){
            delete m_coder;
            m_coder = NULL;
        }
     }

    void TcpConnection::onread(){
        if(m_state!=Connected){
            ERRORLOG("onread error,client has already disconnected,addr [%s],clientfd[%d]",m_peer_addr->toString().c_str(),m_fd);
            return;
        }

        bool is_read_all = false;
        bool is_closed = false;
        while(!is_read_all){
            if(m_in_buffer->writAble()==0){
                m_in_buffer->resizeBuffer(2*m_in_buffer->m_buffer.size());
            }
            int read_count = m_in_buffer->writAble();
            int write_index = m_in_buffer->writeIndex();

            int rt = read(m_fd_event->getfd(),&(m_in_buffer->m_buffer[write_index]),read_count);
            DEBUGLOG("success read %d bytes from addr[%s],clientfd[%d]",rt,m_peer_addr->toString().c_str(),m_fd);
            if(rt>0){
                m_in_buffer->moveWriteIndex(rt);
                if(rt==read_count){
                    continue;
                }else if(rt<read_count){
                    is_read_all = true;
                    break;
                }
            }
            else if(rt==0){
                is_closed = true;
                break;
            }
            else if(rt==-1&&errno ==EAGAIN){
                is_read_all = true;
                break;
            }
        }
        if(is_closed){
            INFOLOG("peer closed,peer addr [%d],clientfd [%d]",m_peer_addr->toString().c_str(),m_fd);
            clear();
            return;
        }
        if(!is_read_all){
            ERRORLOG("not read all data");
        }
        excute();
    }

    void TcpConnection::excute(){
        if(m_connection_type == TcpConnectionByServer){
        std::vector<AbstractProtocol::s_ptr> result;
        std::vector<AbstractProtocol::s_ptr> replay_message;
        m_coder->decode(result,m_in_buffer);
        for(size_t i = 0; i<result.size();i++){
            //针对每一个请求调用rpc方法，获取响应message
            //将响应message放入到发送缓冲区，监听可写事件回包
            INFOLOG("success get request[%s] from client[%s]",result[i]->m_req_id.c_str(),m_peer_addr->toString().c_str());
            std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
            message->m_pb_data = "hello,this is rocket roc test data";
            message->m_req_id = result[i]->m_req_id;
            replay_message.emplace_back(message);
        }
        m_coder->encode(replay_message,m_out_buffer);
        listenWrite();
        }
        else{
            //从buffer里decode得到message对象，判断req_id是否相等，相等则读成功，执行其回调
            std::vector<AbstractProtocol::s_ptr> result;
            m_coder->decode(result,m_in_buffer);

            for(size_t i = 0;i<result.size();i++)
            {
                std::string req_id = result[i]->m_req_id;
                auto it = m_read_dones.find(req_id);
                if(it!=m_read_dones.end()){
                    it->second(result[i]);
                }
            }
        }
    }

    void TcpConnection::onwrite(){
        if(m_state!=Connected){
            ERRORLOG("onwrite error,client has already disconnected,addr [%s],clientfd[%d]",m_peer_addr->toString().c_str(),m_fd);
            return;
        }
        if(m_connection_type == TcpConnectionByClient)
        {
            std::vector<AbstractProtocol::s_ptr> messages;
            for(size_t i = 0; i<m_write_dones.size();i++){
                messages.push_back(m_write_dones[i].first);
            }
            m_coder->encode(messages,m_out_buffer);
        }        
        bool is_write_all = false; 
        while(true){
            if(m_out_buffer->readAble()==0){
                DEBUGLOG("no data need to send to client [%s]",m_peer_addr->toString().c_str());
                is_write_all = true;
                break;
            }
            int write_size = m_out_buffer->readAble();
            //int write_size = m_out_buffer->writAble();
            int read_index = m_out_buffer->readIndex();

            int rt = write(m_fd,&(m_out_buffer->m_buffer[read_index]),write_size);
            if(rt>=write_size){
                DEBUGLOG("no data need to send to client [%s]",m_peer_addr->toString().c_str());
                is_write_all = true;
                break;
            }
            if(rt==-1 &&errno ==EAGAIN){
                //发送缓冲区已满，不能再发送
                //这种情况等下次fd可写再次发送数据即可
                ERRORLOG("write data error,errno ==EAGAIN and rt ==-1");
                break;
            }
        }
        if(is_write_all){
            m_fd_event->cancel(FdEvent::OUT_EVENT);
            m_event_loop->addEpollEvent(m_fd_event);
        }
        if(m_connection_type==TcpConnectionByClient){
            for(size_t i = 0;i<m_write_dones.size();i++){
                m_write_dones[i].second(m_write_dones[i].first);
            }
            m_write_dones.clear();
        }
    }

    void TcpConnection::listenWrite(){
        m_fd_event->listen(FdEvent::OUT_EVENT,std::bind(&TcpConnection::onwrite,this));
        m_event_loop->addEpollEvent(m_fd_event);
    }

    void TcpConnection::listenRead(){
        m_fd_event->listen(FdEvent::IN_EVENT,std::bind(&TcpConnection::onread,this));
        m_event_loop->addEpollEvent(m_fd_event);
    }

    void TcpConnection::setState(const TcpState state)
    {
        m_state = Connected;
    }

    TcpState TcpConnection::getState()
    {
        return m_state; 
    }

    void TcpConnection::clear()
    {
        //处理一些关闭连接后的清理动作
        if(m_state == closed){
            return;
        }
        m_fd_event->cancel(FdEvent::IN_EVENT);
        m_fd_event->cancel(FdEvent::OUT_EVENT);
        m_event_loop->deleteEpollEvent(m_fd_event);
        m_state = closed;
    }

    void TcpConnection::shutdown()
    {
        if(m_state == closed || m_state == NotConnected){
            return;
        }
        m_state = HalfClosing;
        //调用shutdown关闭读写，意味着服务器不再对这个fd进行读写操作，发送FIN报文，触发四次挥手的第一个阶段，当fd发生可读事件，但是可读的数据为0及对端发生了FIN
        ::shutdown(m_fd,SHUT_RDWR);
    }
    void TcpConnection::setConnectionType(TcpConnectionType type){
        m_connection_type = type;
    }

    void TcpConnection::pushSendMessage(AbstractProtocol::s_ptr message,std::function<void(AbstractProtocol::s_ptr)> done){
        m_write_dones.push_back(std::make_pair(message,done));

    }

    void TcpConnection::pushReadMessage(const std::string& req_id,std::function<void(AbstractProtocol::s_ptr)> done)
    {
        m_read_dones.insert(std::make_pair(req_id,done));
    }



}