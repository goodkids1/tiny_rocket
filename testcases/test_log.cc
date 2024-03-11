#include"/home/zxl/rocket/rocket/common/log.h"
#include"/home/zxl/rocket/rocket/common/config.h"
#include<pthread.h>

void* fun(void *)
{
    DEBUGLOG("debug this is thread in %s","fun");
    INFOLOG("info test info log %s","11");
    return NULL;
}

int main()
{
    rocket::Config::SetGlobalConfig("/home/zxl/rocket/conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    pthread_t thread;
    pthread_create(&thread,NULL,fun,NULL);
    DEBUGLOG("test debug log %s","11");
    INFOLOG("test info log %s","11");
    pthread_join(thread,NULL);
    return 0;
}