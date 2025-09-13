#include "playback.hpp"
#include "wav.hpp"
#include <iostream>
#include <thread>
#include <fstream>
#include <alsa/asoundlib.h>
using namespace std;
using namespace std;
// main作为主程序接口 调用其它组件即可
// 这里实现了播放逻辑

// 播放操作后面播放线程调用
void Playback(const string &filename)
{
    ifstream file(filename.c_str(), ios::binary);
    ox::WAVHeader header;
    // 读取出帧头数据
    file.read((char *)&header, 44);
    // 打开pcm设备 -播放模式
    cout << header.sample_rate << endl;
    cout << header.channels << endl;
    ox::AlsaPlayback play("hw:0,0", header.sample_rate, header.channels);
    play.Open();
    // 循环写数据播放音乐
    // alsa 只支持pcm
    // wav格式文件 44字节头跳过传送后续的原始pcm数据

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
        char buffer[1024] = {0};
        file.read(buffer, 1024);
        int len = 0;
        play.WriteFrame((const uint8_t *)buffer, 1024, &len);
        cout << "成功写入%d:" << len << "帧" << endl;
        if (file.eof())
        {
            // wav文件读取完毕
            cout << "音乐播放完毕" << endl;
            break;
        }
    }
}


int main(int argc, char *argv[])
{
    // 打开wav文件
    const string filename(argv[1]);
    Playback(filename);
    exit(0);
}