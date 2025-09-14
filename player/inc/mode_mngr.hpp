#ifndef _MODE_MNGR_HPP_
#define _MODE_MNGR_HPP_
#include "playback.hpp"
#include <atomic>
#include <vector>
#include <string>
#include <thread>
using namespace std;
// 模式管理类

namespace ox
{
    class Mode_Mngr
    {
    public:
        typedef enum PLAY_MODE
        {
            STANDBY         = 0xaa,             //待定
            SUSPEND         = 0xa0,             //播放暂停
            SINGLE_CYCLE    = 0xa1,             //单曲循环
            SHUFFLE_MODE    = 0xa2,             //随机播放
            LIST_LOOP       = 0xa3,             //列表循环
            EXIT            = 0xa4,             //退出
        } PLAY_MODE;

        Mode_Mngr();
        ~Mode_Mngr();

        // 播放工作函数
        void _Play(const string& filename);
        void Play();        
        void Start();
        void Stop();
        vector<string>  m_lists;                  //存储音乐路径
    public:
        PLAY_MODE       m_cur_mode;               //描述当前的工作模式
        PLAY_MODE       m_status;                   //保存上一次切换后的状态
                                                  //当前状态和上一次不一样就需要响应操作
    public:
        atomic<bool>    m_running;                //描述当前系统是否正常运行
        int             m_index;                  //vector的遍历下标
        // 内聚一个play对象 这样就可以避免设备重复打开
        AlsaPlayback    m_playback;
        // 独占性指针持有工作线程
        std::unique_ptr<thread>   work_thread;


    
    };
}
extern ox::Mode_Mngr mngr;

#endif