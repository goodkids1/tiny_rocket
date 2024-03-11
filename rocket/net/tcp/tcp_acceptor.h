#ifndef ROCKET_NET_TCP_TCP_ACCEPTOR_H
#define ROCKET_NET_TCP_TCP_ACCEPTOR_H
#include"./net_addr.h"
#include<memory>

namespace rocket
{
   class TcpAcceptor
   {

    public:
      typedef std::shared_ptr<TcpAcceptor> s_ptr;
      TcpAcceptor(NetAddr::s_ptr local_addr);
      ~TcpAcceptor();
      std::pair<int,NetAddr::s_ptr> accept();
      int GetListenFd();

    
    private:
      NetAddr::s_ptr m_local_addr;
      int m_listenfd {-1};
      int m_family{-1};

   };
}
#endif