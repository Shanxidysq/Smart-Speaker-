#include "mode_mngr.hpp"
#include "playback.hpp"
#include "wav.hpp"
#include <thread>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <random>
using namespace std;
#define CXX_11

// 增加epoll和线程池处理并发

namespace ox
{
    const string Device_name("hw:0,0");
    const int default_rate = 44100;
    const int default_channle = 2;
    Mode_Mngr::Mode_Mngr() : m_playback(Device_name, default_rate, default_channle)
    {
        // 模式设置
        m_cur_mode = SUSPEND;
        m_status = SUSPEND;
        m_index = 0;
        m_running = false;
        // 内部组件线程池，启动即开始就好了

        // 打开设备
        m_playback.Open();
        // 构建Alsaplayback对象
    }

    Mode_Mngr::~Mode_Mngr()
    {
        Stop();
    }

    // 播放wav格式文件
    void Mode_Mngr::_Play(const string &filename)
    {
        ifstream file(filename.c_str(), ios::binary);
        ox::WAVHeader header;
        // 读取出帧头数据
        file.read((char *)&header, 44);
        m_playback.m_channels = header.channels;
        m_playback.m_sample_rate = header.sample_rate;

        // 这里根据wav头设置参数
        m_playback.SetParams();

        if (!file.is_open())
        {
            cerr << "文件：" << filename << "打开失败" << endl;
            exit(-1);
        }
        // 先不检查wav文件帧头 直接读取pcm原始数据
        // 后面这里存在检查wav格式文件头信息等 所以后面这里需要解析wav格式文件
        file.seekg(44, ios::beg);
        while (1)
        {
            // 播放的时候检测curmode
            // 确保外部设置暂停后可以立即响应
            if (m_cur_mode != m_status)
            {
                break;
            }
            char buffer[1024] = {0};
            file.read(buffer, 1024);
            int len = 0;
            bool ret = m_playback.WriteFrame((const uint8_t *)buffer, 1024, &len);
            if (!ret)
            {
                cerr << "音频数据写入存在问题" << endl;
                exit(1);
            }
            if (file.eof())
            {
                // wav文件读取完毕
                cout << "音乐播放完毕" << endl;
                break;
            }
        }
    }
    // 播放线程接口
    void Mode_Mngr::Play()
    {
        // 看系统是否处于运行态
        while (m_running)
        {
            // 暂停播放
            // 播放状态发生变换需要立即响应 增加一个标志保存为保存上一次切换后的状态
            if (m_cur_mode != SUSPEND)
            {
                // 这里会确保一首音乐播放完毕
                _Play(m_lists[m_index]);
                // 状态改变需要切换index寻址方式
                if (m_cur_mode == EXIT)
                {
                    m_status = m_cur_mode;
                    m_index = 0;
                    break;
                }
                if (m_cur_mode == LIST_LOOP)
                {
                    m_status = m_cur_mode;
                    m_index = ++m_index % m_lists.size();
                }
                if (m_cur_mode == SINGLE_CYCLE)
                {
                    m_status = m_cur_mode;
                }
                if (m_cur_mode == SHUFFLE_MODE)
                {
                    m_status = m_cur_mode;
                    std::random_device rd;                   // 1. 真随机种子
                    std::mt19937 gen(rd());                  // 2. 32 位梅森旋转算法
                    std::uniform_int_distribution<int> dist; // 3. 闭区间 [10,99]

                    m_index = dist(gen) % (m_lists.size());
                }
                if (m_cur_mode == PRE)
                {
                    m_status = m_cur_mode;
                    m_index = --m_index % m_lists.size();
                }
                if (m_cur_mode == NEXT)
                {
                    m_status = m_cur_mode;
                    m_index = ++m_index % m_lists.size();
                }
                m_playback.Drain();
            }
        }
    }
    // 启动工作函数
    void Mode_Mngr::Start()
    {
        if (m_running)
        {
            return;
        }
        m_running = true;
#ifdef CXX_14
        // make_unique 是c++14标准
        work_thread = std::make_unique<std::thread>([this]
                                                    { this->Play(); });
#endif
#ifdef CXX_11
        // 兼容c++11处理
        //
        work_thread.reset(new std::thread([this]
                                          { this->Play(); }));
#endif
    }

    void Mode_Mngr::Stop()
    {
        m_running = false;
        m_cur_mode = SUSPEND;
        if (work_thread && work_thread->joinable())
        {
            work_thread->join();
            work_thread.reset();
        }
    }
}
