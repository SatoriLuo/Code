main.cpp是一个回射服务器的代码
工作原理是为每一个连接套接字fork一个子进程去处理客户
发过来的文字并且回射给客户
该版本有待优化。
