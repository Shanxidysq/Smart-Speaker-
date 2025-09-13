#ifndef _WAV_HPP_
#define _WAV_HPP_
// 描述wav文件格式的头文件信息用于解析wav文件格式使用
#include <iostream>
#include <cstring>
namespace ox
{
    // 强制帧头1字节对齐避免编译器进行了结构体内存对齐优化
#pragma pack(push, 1)
    // wav格式帧头
    class WAVHeader
    {
    public:
        // 按照wav帧头格式设备 memcpy 拷贝44字节内存即可访问帧头数据
        uint8_t riff[4] = {};
        uint32_t file_size = 0;
        uint8_t wave[4] = {};
        uint8_t fmt_id[4] = {};
        uint32_t fmt_size = 0;
        uint16_t format_tag = 0;
        uint16_t channels = 0;
        uint32_t sample_rate = 0;
        uint32_t byte_rate = 0;
        uint16_t block_align = 0;
        uint16_t bits_per_sample = 0;
        uint8_t data_id[4] = {};
        uint32_t data_size = 0;

        WAVHeader()
        {
            memcpy(riff, "RIFF", 4);
            memcpy(wave, "WAVE", 4);
            memcpy(fmt_id, "fmt ", 4);
            memcpy(data_id, "data", 4);
            fmt_size = 16;
            format_tag = 1;
        }
    };
#pragma pack(pop)
}

#endif