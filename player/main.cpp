#include "playback.hpp"
#include "wav.hpp"
#include "mode_mngr.hpp"
#include <iostream>
#include <thread>
#include <fstream>
#include <functional>
#include <alsa/asoundlib.h>
using namespace std;
using namespace std;

// 本质上就是线程间的共享内存通信
// 定义全局变量模式管理类
// 模式管理内聚playback类 管理播放功能，内聚线程池避免多进程等无效浪费资源的逻辑
// 后续需要增加socket通信类
// 增加socket之后就需要epoll监听网络套接字的状态
// 可以说mngr是两个线程 一个socket线程还是使用epoll来监听 这里使用epoll好一点吧

string name2("../music/jiaohuanyusheng.wav");
string name1("../music/daoxiang.wav");

// main作为主程序接口 调用其它组件即可
// 这里实现了播放逻辑

void menu()
{
    cout << "0.SUSPEND" << endl;
    cout << "1.SINGLE_CYCLE" << endl;
    cout << "2.SHUFFLE_MODE" << endl;
    cout << "3.LIST_LOOP" << endl;
    cout << "4.break" << endl;
}

int main(int argc, char *argv[])
{
    ox::Mode_Mngr mngr;

    mngr.m_lists.push_back(name2);
    mngr.m_lists.push_back(name1);
    mngr.m_cur_mode = ox::Mode_Mngr::SINGLE_CYCLE;
    mngr.m_status   = ox::Mode_Mngr::SINGLE_CYCLE;

    mngr.Start();

    while (1)
    {
        menu();
        int options;
        // 等待用户输入后再退出

        std::cout << "cin you select" << std::endl;
        cin >> options;
        switch (options)
        {
        case 0:
            mngr.m_cur_mode = ox::Mode_Mngr::SUSPEND;
            break;
        case 1:
            mngr.m_cur_mode = ox::Mode_Mngr::SINGLE_CYCLE;
            break;
        case 2:
            mngr.m_cur_mode = ox::Mode_Mngr::SHUFFLE_MODE;
            break;
        case 3:
            mngr.m_cur_mode = ox::Mode_Mngr::LIST_LOOP;
            break;
        }
        if(mngr.m_cur_mode == ox::Mode_Mngr::EXIT)
        {
            break;
        }
    }

    mngr.Stop();
    return 0;
}