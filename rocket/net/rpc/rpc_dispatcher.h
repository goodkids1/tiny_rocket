#ifndef ROCKET_NET_RPC_RPC_DISPATCHER_H
#define ROCKET_NET_RPC_RPC_DISPATCHER_H

#include"/home/zxl/rocket/rocket/net/coder/abstract_protocol.h"
#include"/home/zxl/rocket/rocket/net/coder/tinypb_protocol.h"
#include<map>
#include<memory>
#include<google/protobuf/service.h>


namespace rocket{

class RpcDispatcher{

    public:
    typedef std::shared_ptr<google::protobuf::Service> service_s_ptr;
    void dispatch(AbstractProtocol::s_ptr request,AbstractProtocol::s_ptr response);

    void registerService(service_s_ptr service);

    void serTinyPBError(std::shared_ptr<TinyPBProtocol> msg,int32_t err_code,const std::string err_info);

    private:
    bool parseServiceFullName(const std::string& full_name,std::string& service_name,std::string& methood_name);

    private:
    std::map<std::string,service_s_ptr> m_service_map;

};


}







#endif