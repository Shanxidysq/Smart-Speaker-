#ifndef EPOLL_SERVER_HPP
#define EPOLL_SERVER_HPP

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace ox
{
    // 连接类
    class Connect
    {
    public:
        static const int read_size = 1024;
        static const int write_size = 1024;

    public:
        int fd;
        // 非阻塞套接字的读写缓冲区
        char read_buffer[read_size];
        char write_buffer[write_size];
        int read_len;
        int write_len;
        bool want_write;

        Connect(int fd);
        ~Connect();

        // 禁止拷贝
        Connect(const Connect &con) = delete;
        Connect &operator=(const Connect &con) = delete;

        // 允许移动
        Connect(Connect &&other) noexcept;
        Connect &operator=(Connect &&other) noexcept;
    };

    // epoll 服务器
    class EpollServer
    {
    private:
        // server_fd 监听套接字
        int server_fd;
        // epoll_fd 描述epoll的内核事件表的描述符
        int epoll_fd;
        bool running;
        std::unordered_map<int, std::unique_ptr<Connect>> connections;

        // 内部辅助方法
        bool set_nonblocking(int fd);
        bool epoll_ctl_op(int op, int fd, uint32_t events, Connect *con);

        // 事件处理方法
        void handle_accept();
        void handle_read(Connect *con);
        void handle_write(Connect *con);
        void handle_close(Connect *con);
        void handle_error(Connect *conn);

    public:
        EpollServer();
        ~EpollServer();

        // 禁止拷贝
        EpollServer(const EpollServer &) = delete;
        EpollServer &operator=(const EpollServer &) = delete;

        // 允许移动
        EpollServer(EpollServer &&other) noexcept;
        EpollServer &operator=(EpollServer &&other) noexcept;

        // 服务器控制方法
        bool start(int port);
        void run();
        void stop();

        // 状态查询
        bool is_running() const { return running; }
        size_t connection_count() const { return connections.size(); }
    };

}

#endif