#include "playback.hpp"
#include "wav.hpp"
#include "mode_mngr.hpp"
#include "epoll.hpp"
#include <iostream>
#include <thread>
#include <fstream>
#include <functional>
#include <alsa/asoundlib.h>
using namespace std;

ox::Mode_Mngr mngr;


string name1("../music/daoxiang.wav");
string name2("../music/jiaohuanyusheng.wav");
string name3("../music/10s.wav");
string name4("../music/woshirucixiangxin.wav");
string name5("../music/huahai.wav");
string name6("../music/qinghuaci.wav");
string name7("../music/qingtian.wav");
string name8("../music/gaibaoqiqiu.wav");

void menu()
{
    cout << "0.SUSPEND" << endl;
    cout << "1.SINGLE_CYCLE" << endl;
    cout << "2.SHUFFLE_MODE" << endl;
    cout << "3.LIST_LOOP" << endl;
    cout << "4.PRE" << endl;
    cout << "5.NEXT" << endl;
    cout << "9.break" << endl;
}

void func(ox::Mode_Mngr &mngr)
{
    while (1)
    {
        menu();
        int options;
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
        case 4:
            mngr.m_cur_mode = ox::Mode_Mngr::PRE;
            mngr.m_status = ox::Mode_Mngr::STANDBY;
            break;
        case 5:
            mngr.m_cur_mode = ox::Mode_Mngr::NEXT;
            mngr.m_status = ox::Mode_Mngr::STANDBY;
            break;
        }
        if (options == 9)
        {
            break;
        }
    }
}

int main(int argc, char *argv[])
{

    mngr.m_lists.push_back(name2);
    mngr.m_lists.push_back(name1);
    mngr.m_lists.push_back(name3);
    mngr.m_lists.push_back(name4);
    mngr.m_lists.push_back(name5);
    mngr.m_lists.push_back(name6);
    mngr.m_lists.push_back(name7);
    mngr.m_lists.push_back(name8);

    mngr.m_cur_mode = ox::Mode_Mngr::SINGLE_CYCLE;
    mngr.m_status = ox::Mode_Mngr::SINGLE_CYCLE;

    mngr.Start();
    mngr.m_threadpools->add([]()
                            { func(mngr); });

    // 启动epoll-server
    ox::EpollServer server;
    if (!server.start(9898))
    {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }

    std::cout << "Epoll server running on port 9898. Press Ctrl+C to stop." << std::endl;
    server.run();

    return 0;
}