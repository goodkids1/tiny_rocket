#ifndef ROCKET_NET_CODER_TINYPB_CODER_H
#define ROCKET_NET_CODER_TINYPB_CODER_H

#include"./abstract_coder.h"
#include"./tinypb_protocol.h"
#include<string>

namespace rocket{
class TinyPBCoder : public AbstractCoder{
    public:
    //将message对象转化为字节流写入到buffer
    void encode(std::vector<AbstractProtocol::s_ptr>& messages,TcpBuffer::s_ptr out_buffer);

    //将buffer里面的字节流转换为message对象
    void decode(std::vector<AbstractProtocol::s_ptr>& out_messages,TcpBuffer::s_ptr buffer);  
    TinyPBCoder(){
        
    }
     
    ~TinyPBCoder(){

    }

    private:
    
    const char* encodeTinyPB(std::shared_ptr<TinyPBProtocol> message,int &len);
};
}







#endif