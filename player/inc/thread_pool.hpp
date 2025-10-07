#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__
// 线程
#include <thread>
// 锁
#include <mutex>
// 原子操作
#include <atomic>
// 条件变量
#include <condition_variable>
// 可调用函数的包装
#include <functional>
// 异步结果返回
#include <future>
// 队列
#include <queue>
// 类型萃取器
#include <type_traits>
// 功能组
#include <utility>
// 容器[数组]
#include <vector>
// thread pool class design

// task
// 复习 function
//      future
//
namespace ox
{
    // 线程池管理
    class ThreadsGuard
    {

    public:
        // 初始化列表赋值
        ThreadsGuard(std::vector<std::thread> &threads);
        ~ThreadsGuard();

    private:
        // 删除移动构造移动赋值函数
        ThreadsGuard(ThreadsGuard &&tg) = delete;
        ThreadsGuard &operator=(ThreadsGuard &&tg) = delete;
        // 拷贝构造拷贝赋值函数删除
        ThreadsGuard(const ThreadsGuard &) = delete;
        ThreadsGuard &operator=(const ThreadsGuard &) = delete;

    private:
        // 线程池
        std::vector<std::thread> &m_threads;
    };

    class ThreadPool
    {
    public:
        // 定义任务类型
        typedef std::function<void()> task_type;

    public:
        // 避免隐式类型转换
        ThreadPool(int n = 0);

        ~ThreadPool();

        void stop();
        //通过函数模板添加任务到任务队列
        // function 函数指针 任务实体
        // Args 函数参数
    // 添加任务
    template <class Function, class... Args>
    std::future<typename std::result_of<Function(Args...)>::type> add(Function &&fcn, Args &&...args)
    {
        // 返回值类型的萃取 和参数类型
        typedef typename std::result_of<Function(Args...)>::type return_type;
        // 异步获取返回值 任务包装器
        typedef std::packaged_task<return_type()> task;

        // 完美转发 实现任务的添加
        // shared_ptr 管理任务
        // bin 函数体和函数参数的绑定

        std::shared_ptr<task> t = std::make_shared<task>(std::bind(std::forward<Function>(fcn),
                                                                   std::forward<Args>(args)...));
        // 返回值的绑定
        auto ret = t->get_future();
        {
            // 获取锁 线程池的同步
            std::lock_guard<std::mutex> lg(m_mtx);
            // 检查线程池状态
            if (m_stop.load(std::memory_order_acquire))
            {
                throw std::runtime_error("thread pool has stopped");
            }
            //原位构建
            m_tasks.emplace([t]
                           {
                            //这里创建一个lambda右值对象
                            //捕获近来
                            //在线程中执行
                            (*t)(); }); // task
        }
        //通知一个cv 阻塞队列里面的线程 一次添加了一个任务
        //避免惊群效应
        m_cond.notify_one();
        return ret;
    }

    private:
        // 删除移动赋值 移动构造函数 避免人物被窃取
        // 删除拷贝构造拷贝赋值汉书 避免多个线程持有一个任务
        ThreadPool(ThreadPool &&) = delete;
        ThreadPool &operator=(ThreadPool &&) = delete;
        ThreadPool(const ThreadPool &) = delete;
        ThreadPool &operator=(const ThreadPool &) = delete;

    private:
        // atomic 原子操作
        std::atomic<bool> m_stop;
        // 锁
        std::mutex m_mtx;
        // 条件变量
        std::condition_variable m_cond;

        // 任务队列
        std::queue<task_type> m_tasks;
        // 线程池
        std::vector<std::thread> m_threads;
        // 线程管理
        // 利用RAII机制 确保每个线程都可以正确结束
        ox::ThreadsGuard m_tg;
    };

}

#endif