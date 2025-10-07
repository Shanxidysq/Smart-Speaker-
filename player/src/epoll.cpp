#include "epoll.hpp"
#include "mode_mngr.hpp"
#include <cctype>

namespace ox
{

    // Connect 类实现
    Connect::Connect(int fd) : fd(fd),
                               read_len(0),
                               write_len(0),
                               want_write(false)
    {
        memset(read_buffer, 0, read_size);
        memset(write_buffer, 0, write_size);
    }

    Connect::~Connect()
    {
        if (fd >= 0)
        {
            close(fd);
        }
    }

    Connect::Connect(Connect &&other) noexcept : fd(other.fd),
                                                 read_len(other.read_len),
                                                 write_len(other.write_len),
                                                 want_write(other.want_write)
    {
        memcpy(read_buffer, other.read_buffer, read_size);
        memcpy(write_buffer, other.write_buffer, write_size);
        other.fd = -1; // 防止原对象关闭文件描述符
    }

    Connect &Connect::operator=(Connect &&other) noexcept
    {
        if (this != &other)
        {
            // 先清理当前资源
            if (fd >= 0)
            {
                close(fd);
            }

            // 转移资源
            fd = other.fd;
            read_len = other.read_len;
            write_len = other.write_len;
            want_write = other.want_write;
            memcpy(read_buffer, other.read_buffer, read_size);
            memcpy(write_buffer, other.write_buffer, write_size);

            // 置空原对象
            other.fd = -1;
        }
        return *this;
    }

    // EpollServer 私有方法实现
    bool EpollServer::set_nonblocking(int fd)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1)
        {
            perror("fcntl F_GETFL");
            return false;
        }
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        {
            perror("fcntl F_SETFL");
            return false;
        }
        return true;
    }

    bool EpollServer::epoll_ctl_op(int op, int fd, uint32_t events, Connect *con)
    {
        struct epoll_event ev;
        ev.events = events;
        ev.data.ptr = con;

        if (epoll_ctl(epoll_fd, op, fd, &ev) == -1)
        {
            perror("epoll_ctl error");
            return false;
        }
        return true;
    }

    // 连接处理
    void EpollServer::handle_accept()
    {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);

        int client_fd = accept4(server_fd, (struct sockaddr *)&client_addr,
                                &len, SOCK_NONBLOCK);
        if (client_fd == -1)
        {
            perror("accept4 error");
            return;
        }

        printf("New connection from: %s port: %d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        auto con = std::make_unique<Connect>(client_fd);

        if (!epoll_ctl_op(EPOLL_CTL_ADD, client_fd,
                          EPOLLIN | EPOLLET | EPOLLRDHUP, con.get()))
        {
            close(client_fd);
            return;
        }

        connections[client_fd] = std::move(con);
    }

    // 读事件处理
    void EpollServer::handle_read(Connect *con)
    {
        while (true)
        {
            ssize_t count = read(con->fd, con->read_buffer + con->read_len,
                                 Connect::read_size - con->read_len);
            // 接收到数据之后处理处理，解析

            if (count > 0)
            {
                con->read_len += count;

                // 查找是否收到完整的一行（以换行符结尾）
                for (int i = 0; i < con->read_len; ++i)
                {
                    if (con->read_buffer[i] == '\n')
                    {
                        // 处理收到的数据
                        int line_length = i + 1;
                        for (int j = 0; j < line_length - 1; ++j)
                        {
                            con->write_buffer[j] = std::toupper(con->read_buffer[j]);
                        }
                        con->write_buffer[line_length - 1] = '\n';
                        con->write_len = line_length;

                        // 移除已处理的数据
                        memmove(con->read_buffer, con->read_buffer + line_length,
                                con->read_len - line_length);
                        con->read_len -= line_length;

                        // 尝试写入响应
                        handle_write(con);
                        break;
                    }
                }
            }
            else if (count == 0)
            {
                // 对方关闭连接
                std::cout << "Connection closed by client" << std::endl;
                handle_close(con);
                return;
            }
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    break;
                }
                else
                {
                    perror("read error");
                    handle_close(con);
                    return;
                }
            }
        }
    }

    // 处理写
    void EpollServer::handle_write(Connect *con)
    {
        if (con->write_len <= 0)
        {
            if (con->want_write)
            {
                con->want_write = false;
                epoll_ctl_op(EPOLL_CTL_MOD, con->fd,
                             EPOLLIN | EPOLLET | EPOLLRDHUP, con);
            }
            return;
        }

        ssize_t count = write(con->fd, con->write_buffer, con->write_len);

        if (count > 0)
        {
            con->write_len -= count;
            if (con->write_len > 0)
            {
                memmove(con->write_buffer, con->write_buffer + count, con->write_len);
            }

            if (con->write_len > 0)
            {
                if (!con->want_write)
                {
                    con->want_write = true;
                    epoll_ctl_op(EPOLL_CTL_MOD, con->fd,
                                 EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP, con);
                }
            }
            else
            {
                if (con->want_write)
                {
                    con->want_write = false;
                    epoll_ctl_op(EPOLL_CTL_MOD, con->fd,
                                 EPOLLIN | EPOLLET | EPOLLRDHUP, con);
                }
            }
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                if (!con->want_write)
                {
                    con->want_write = true;
                    epoll_ctl_op(EPOLL_CTL_MOD, con->fd,
                                 EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP, con);
                }
            }
            else
            {
                perror("write error");
                handle_close(con);
            }
        }
    }

    void EpollServer::handle_close(Connect *con)
    {
        connections.erase(con->fd);
    }

    void EpollServer::handle_error(Connect *conn)
    {
        std::cerr << "Error occurred on connection" << std::endl;
        handle_close(conn);
    }

    // EpollServer 公有方法实现
    EpollServer::EpollServer() : server_fd(-1),
                                 epoll_fd(-1),
                                 running(false)
    {
    }

    EpollServer::~EpollServer()
    {
        stop();
    }

    EpollServer::EpollServer(EpollServer &&other) noexcept : server_fd(other.server_fd),
                                                             epoll_fd(other.epoll_fd),
                                                             running(other.running),
                                                             connections(std::move(other.connections))
    {
        other.server_fd = -1;
        other.epoll_fd = -1;
        other.running = false;
    }

    EpollServer &EpollServer::operator=(EpollServer &&other) noexcept
    {
        if (this != &other)
        {
            // 清理当前资源
            stop();

            // 转移资源
            server_fd = other.server_fd;
            epoll_fd = other.epoll_fd;
            running = other.running;
            connections = std::move(other.connections);

            // 置空原对象
            other.server_fd = -1;
            other.epoll_fd = -1;
            other.running = false;
        }
        return *this;
    }

    bool EpollServer::start(int port)
    {
        server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (server_fd == -1)
        {
            perror("socket error");
            return false;
        }

        int option = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
        {
            perror("setsockopt error");
            close(server_fd);
            return false;
        }

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        {
            perror("bind error");
            close(server_fd);
            return false;
        }

        if (listen(server_fd, SOMAXCONN) == -1)
        {
            perror("listen error");
            close(server_fd);
            return false;
        }

        epoll_fd = epoll_create1(0);
        if (epoll_fd == -1)
        {
            perror("epoll_create error");
            close(server_fd);
            return false;
        }

        // 创建服务器连接的Connect对象
        auto server_con = std::make_unique<Connect>(server_fd);

        if (!epoll_ctl_op(EPOLL_CTL_ADD, server_fd, EPOLLIN | EPOLLET, server_con.get()))
        {
            close(server_fd);
            close(epoll_fd);
            return false;
        }

        connections[server_fd] = std::move(server_con);

        running = true;
        std::cout << "Server started on port " << port << std::endl;
        return true;
    }

    void EpollServer::run()
    {
        const int MAX_EVENTS = 64;
        struct epoll_event events[MAX_EVENTS];

        while (running)
        {
            int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
            if (nfds == -1)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                perror("epoll_wait");
                break;
            }

            for (int i = 0; i < nfds; ++i)
            {
                Connect *conn = static_cast<Connect *>(events[i].data.ptr);

                // 检查是否是服务器socket
                if (conn->fd == server_fd)
                {
                    // 移交线程池处理
                    mngr.m_threadpools->add([this]
                                            { this->handle_accept(); });

                    continue;
                }

                uint32_t event_mask = events[i].events;

                if (event_mask & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
                {
                    mngr.m_threadpools->add([this,&conn]
                                            { this->handle_error(conn); });
                    continue;
                }

                if (event_mask & EPOLLIN)
                {
                    mngr.m_threadpools->add([this,&conn]
                                            { this->handle_read(conn); });
                }

                if (event_mask & EPOLLOUT)
                {
                    mngr.m_threadpools->add([this,&conn]
                                            { this->handle_write(conn); });
                }
            }
        }
    }

    void EpollServer::stop()
    {
        running = false;
        if (epoll_fd >= 0)
        {
            close(epoll_fd);
            epoll_fd = -1;
        }
        if (server_fd >= 0)
        {
            close(server_fd);
            server_fd = -1;
        }
        connections.clear();
    }

}