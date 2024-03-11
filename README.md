日志模块：
'''
1.日志级别
2.打印到文件，支持日期命名，以及日志的滚动。
3.c 格式风控
4.线程安全
'''
loglevel:
'''
Debug
Info
Error
'''

Logevent:
'''
文件号、行号
MsgNo
进程号
线程id
日期以及时间 精确到ms
自定义消息
'''
日志格式：[level][%y-%m-%d %H:%M:%s.%ms]\t[pid:thread_id]\t[file_name:line][%msg]

logger  日志器
1.提供打印日志的方法
2.设置输出路径 


'''
RPC服务端流程
'''
启动的时候就注册OrderService对象
1.从buffer读取数据 然后decode得到请求的TinyPBProtocol对象，然后从请求的TinyPBProtocol得到method_name 从OrderService对象根据service.method_name
找到方法func
2.找到对应的request type以及response type
3.将请求体TinyPBProtocol 里的pb_data反序列化为request type的一个对象，声明一个空的response type对象
4.func(request,response)
5.将response对象序列化为pb_data 再塞入到TinyPBProtocol结构体中 做encode然后塞入到buffer里面 就会发送回包了。