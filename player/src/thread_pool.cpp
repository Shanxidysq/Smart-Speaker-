#include "thread_pool.hpp"

namespace ox
{
    ThreadsGuard::ThreadsGuard(std::vector<std::thread> &threads)
        : m_threads(threads)
    {
        // 引用必须在初始化列表里面赋值吗
        //m_threads = threads;
        // 构造函数体（通常不需要额外操作）
    }

    ThreadsGuard::~ThreadsGuard()
    {
        for (auto &x : m_threads)
        {
            if (x.joinable())
            {
                // 优雅的等待线程结束
                x.join();
            }
        }
    }

    // 构造函数
    ThreadPool::ThreadPool(int n ) : m_stop(false), m_tg(m_threads)
    {
        int nthreads = n;
        // 没有实际的线程参数
        if (nthreads <= 0)
        {
            // 获取cpu最大支持的并发线程书
            nthreads = std::thread::hardware_concurrency();
            // 如果不支持 2
            // 支持 就按照最大并发数量来创建
            nthreads = nthreads == 0 ? 2 : nthreads;
        }

        // 根据nthreads创建线程
        for (int i = 0; i < nthreads; ++i)
        {
            // push_back 尾插
            //  ([this]())lambda表达式
            m_threads.push_back(std::thread([this]
                                            {
                //线程池里面每个线程的task 主循环拿任务然后执行任务
                //memory_order_acquire 直接读内存 避免 冲突
                //判断线程池有没有停止
                while(!m_stop.load(std::memory_order_acquire))
                {
                    task_type task;
                    {
                        //获取线程池的锁
                        //避免vector的访问冲突
                        std::unique_lock<std::mutex> lock(this->m_mtx);
                        //这里c++的条件变量
                        //c不满足条件会阻塞
                        //线程池没有停止 或者 任务队列非空 并且lock不互斥
                        //如果没有获取到锁会添加到条件变量的阻塞队列之中
                        //这里不满足会休眠 不会自旋消耗cpu资源
                        this->m_cond.wait(lock,[this]{
                            return this->m_stop.load(std::memory_order_acquire) 
                                || (!m_tasks.empty());
                        });
                        //这里如果线程池停止了 就退出
                        if(this->m_stop.load(std::memory_order_acquire))
                        {
                            return ;
                        }
                        //取任务
                        task = std::move(this->m_tasks.front());
                        //出队
                        this->m_tasks.pop();
                    }
                    //执行任务
                    task();
                } }));
        }
    }
    void ThreadPool::stop()
    {
        //添加 停止标志
        m_stop.store(true, std::memory_order_release);
    }
    ThreadPool::~ThreadPool()
    {
        //执行stop
        stop();
        //通知每一个线程
        m_cond.notify_all();
    }
};