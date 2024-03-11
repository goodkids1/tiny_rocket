#include"/home/zxl/rocket/rocket/common/log.h"
#include"/home/zxl/rocket/rocket/net/tcp/net_addr.h"
#include"/home/zxl/rocket/rocket/net/tcp/tcp_server.h"
#include<memory>

void test_tcp_server()
{
    rocket::IPNetAddr::s_ptr addr= std::make_shared<rocket::IPNetAddr>("127.0.0.1",12346);
    DEBUGLOG("create addr %s",addr->toString().c_str());
    rocket::TcpServer tcp_server(addr);
    tcp_server.start();
}

int main()
{
    rocket::Config::SetGlobalConfig("/home/zxl/rocket/conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    test_tcp_server();
    return 0;

}