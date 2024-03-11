#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H
#include<string>
#include<queue>
#include<memory>
#include"./config.h"
#include"./mutex.h"

namespace rocket{

#define DEBUGLOG(str,...)\
    if(rocket::Logger::GetGlobalLogger()->getLogLevel()<=rocket::Debug)\
    {\
    rocket::Logger::GetGlobalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Debug).toString()+"["+std::string(__FILE__)+":"+std::to_string(__LINE__)+"]\t"+rocket::formatString(str,##__VA_ARGS__)+"\n");\     
    rocket::Logger::GetGlobalLogger()->log();                                                                                                            \
    }\     

#define INFOLOG(str,...)\
if(rocket::Logger::GetGlobalLogger()->getLogLevel()<=rocket::Info)\
{\
    rocket::Logger::GetGlobalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Info).toString()+"["+std::string(__FILE__)+":"+std::to_string(__LINE__)+"]\t"+rocket::formatString(str,##__VA_ARGS__)+"\n");\
    rocket::Logger::GetGlobalLogger()->log();                                                                                                                                                      \
}\

#define ERRORLOG(str,...)\
if(rocket::Logger::GetGlobalLogger()->getLogLevel()<=rocket::Error)\
{\
    rocket::Logger::GetGlobalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Error).toString()+"["+std::string(__FILE__)+":"+std::to_string(__LINE__)+"]\t"+rocket::formatString(str,##__VA_ARGS__)+"\n");\                                                 
    rocket::Logger::GetGlobalLogger()->log();                                                                                                                                                       \
}\

#define UNKNOWNLOG(str,...)\
if(rocket::Logger::GetGlobalLogger()->getLogLevel()<=rocket::Debug)\
{\
    rocket::Logger::GetGlobalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Unknown).toString()+"["+std::string(__FILE__)+":"+std::to_string(__LINE__)+"]\t"+rocket::formatString(str,##__VA_ARGS__)+"\n");\                
    rocket::Logger::GetGlobalLogger()->log();                                                                             \
}\

enum LogLevel{
        Unknown = 0,
        Debug =1,
        Info =2,
        Error =3
};

class Logger{
    public:
    void pushLog(const std::string& msg);
    Logger(LogLevel level):m_set_level(level){}
    typedef std::shared_ptr<Logger> s_ptr;
    void log();
    public:
    static Logger* GetGlobalLogger();
    LogLevel getLogLevel() const{
        return m_set_level;
    }
    static void InitGlobalLogger();
     
    private:
    LogLevel m_set_level;
    std::queue<std::string> m_buffer;
    Mutex m_mutex;

};
LogLevel StringToLogLevel(const std::string &log_level);

template<typename... Args>
std::string formatString(const char*str,Args&&... args)
{
    int size = snprintf(nullptr,0,str,args...);
    std::string result;
    if(size>0)
    {
        result.resize(size);
        snprintf(&result[0],size+1,str,args...);
    }
    return result;
}

std::string LogLevelToString(LogLevel level);

class LogEvent{
public:
std::string getFileName()const{
    return m_file_name;
}
LogEvent(LogLevel level):m_level(level){

}

LogLevel getLogLevel()
{
    return m_level;
}
std::string toString();
private:
std::string m_file_name; //文件名
int32_t m_pid;//进程号
int32_t m_file_line;//行号
int32_t m_thread_id;//线程号
LogLevel m_level; //日志级别
Logger::s_ptr m_logger;
};
}
#endif
