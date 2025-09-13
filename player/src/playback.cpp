#include "playback.hpp"
#include "wav.hpp"
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
using namespace std;
// #define QUERY

namespace ox
{
    // 一个完整的播放器 自适应参数应该修改一下这一点
    //  名字空间域下搭建
    //  默认构造函数
    AlsaPlayback::AlsaPlayback(const std::string device, const int sample_rate,
                               const int channels)
        : m_device(device), m_sample_rate(sample_rate), m_channels(channels),
          m_pcm_handle(nullptr), m_format(SND_PCM_FORMAT_S16_LE)
    {
    }
    // 析构函数
    AlsaPlayback::~AlsaPlayback()
    {
        // 关闭设备
        Close();
    }
    // 打开设备
    bool AlsaPlayback::Open()
    {
        // 检查是否已经打开
        if (m_pcm_handle)
        {
            return true;
        }

        // 打开ALSA设备
        int err = snd_pcm_open(&m_pcm_handle, m_device.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
        if (err < 0)
        {
            std::cerr << "无法打开音频设备: " << m_device << ": " << snd_strerror(err) << std::endl;
            return false;
        }

        // 设置音频参数
        if (!SetParams())
        {
            Close();
            return false;
        }

        // 准备播放
        err = snd_pcm_prepare(m_pcm_handle);
        if (err < 0)
        {
            std::cerr << "无法准备播放: " << snd_strerror(err) << std::endl;
            Close();
            return false;
        }

        std::cout << "音频播放初始化完成: " << m_device << std::endl;
        return true;
    }
    // 关闭设备
    void AlsaPlayback::Close()
    {
        if (nullptr != m_pcm_handle)
        {
            // 释放资源
            snd_pcm_close(m_pcm_handle);
            m_pcm_handle = nullptr;
            std::cout << " pcm设备已经关闭 " << std::endl;
        }
    }
    // 缓冲区中写数据
    bool AlsaPlayback::WriteFrame(const uint8_t *buffer, size_t buffer_size, int *frames_written)
    {
        if (nullptr == m_pcm_handle)
        {
            std::cerr << " 设备未打开 " << std::endl;
            return false;
        }
        /* 需要降混？ */
        if (m_downmix)
        {
            static thread_local std::vector<uint8_t> stereo;
            stereo.resize(buffer_size);
            std::memcpy(stereo.data(), buffer, buffer_size);
            buffer_size = Downmix6to2(reinterpret_cast<int16_t *>(stereo.data()),
                                      buffer_size);
            buffer = stereo.data();
        }
        // 计算可以写入的最大帧数
        int max_frames = buffer_size / (m_channels * GetBytesPerSample());

        // 写入音频帧
        int result = snd_pcm_writei(m_pcm_handle, buffer, max_frames);

        if (result < 0)
        {
            std::cerr << "写入音频帧失败: " << snd_strerror(result) << std::endl;
            return false;
        }

        if (frames_written)
        {
            *frames_written = result;
        }

        return true;
    }
    // 恢复设备
    bool AlsaPlayback::Recover(int err)
    {
        if (nullptr != m_pcm_handle)
        {
            std::cerr << "设备已经打开无法恢复" << std::endl;
            return false;
        }
        int ret = snd_pcm_recover(m_pcm_handle, err, 0);
        if (ret < 0)
        {
            std::cout << " 设备恢复失败 " << std::endl;
            return false;
        }
        return true;
    }

    // 降低混音
    // 把 6ch interleaved S16_LE 降混成 2ch，结果就地覆盖原 buffer
    // 返回：新的字节数（= 原大小 / 3）
    // c++11 不支持slamp裁减操作
    template <class T>
    inline constexpr T clamp(const T &v, const T &lo, const T &hi)
    {
        return std::max(lo, std::min(v, hi));
    }
    size_t AlsaPlayback::Downmix6to2(int16_t *buf, size_t bytes_in)
    {
        const size_t samples_in = bytes_in / sizeof(int16_t);
        const size_t frames_in = samples_in / 6;
        const size_t samples_out = frames_in * 2;

        const float scale = 0.707f;
        for (size_t i = 0; i < frames_in; ++i)
        {
            int16_t *in = buf + i * 6;
            int16_t *out = buf + i * 2;

            int32_t L = in[0] + scale * in[2] + scale * (in[4] + in[5]);
            int32_t R = in[1] + scale * in[2] + scale * (in[4] + in[5]);

            out[0] = clamp(L, -32768, 32767);
            out[1] = clamp(R, -32768, 32767);
        }
        return samples_out * sizeof(int16_t);
    }

    bool AlsaPlayback::SetParams()
    {
        snd_pcm_hw_params_t *params;
        // alloca分配栈空间
        snd_pcm_hw_params_alloca(&params);

        /* 1. 初始化参数结构 */
        int err = snd_pcm_hw_params_any(m_pcm_handle, params);
        if (err < 0)
        {
            std::cerr << "无法初始化硬件参数: " << snd_strerror(err) << std::endl;
            return false;
        }

        /* 2. 访问类型 */
        err = snd_pcm_hw_params_set_access(m_pcm_handle, params,
                                           SND_PCM_ACCESS_RW_INTERLEAVED);
        if (err < 0)
        {
            std::cerr << "无法设置访问类型: " << snd_strerror(err) << std::endl;
            return false;
        }

        /* 3. 采样格式 */
        err = snd_pcm_hw_params_set_format(m_pcm_handle, params, m_format);
        if (err < 0)
        {
            std::cerr << "无法设置采样格式: " << snd_strerror(err) << std::endl;
            return false;
        }

        /* 4. 通道数：先 6，不行就 2 */
        err = snd_pcm_hw_params_set_channels(m_pcm_handle, params, m_channels);
        if (err == -EINVAL)
        { // 硬件不支持 6ch
            err = snd_pcm_hw_params_set_channels(m_pcm_handle, params, 2);
            if (err < 0)
            {
                std::cerr << "无法设置 2 通道: " << snd_strerror(err) << std::endl;
                return false;
            }
            m_channels = 2;
            m_downmix = true; // 标记需要软件降混
        }
        else if (err < 0)
        { // 其他错误
            std::cerr << "无法设置通道数: " << snd_strerror(err) << std::endl;
            return false;
        }
        else
        { // 6ch 成功
            m_channels = m_channels;
            m_downmix = false;
        }

        /* 5. 采样率 */
        unsigned int rate = m_sample_rate;
        err = snd_pcm_hw_params_set_rate_near(m_pcm_handle, params, &rate, 0);
        if (err < 0)
        {
            std::cerr << "无法设置采样率: " << snd_strerror(err) << std::endl;
            return false;
        }
        m_sample_rate = rate;

        /* 6. 缓冲区大小（100 ms） */
        snd_pcm_uframes_t buf_frames = m_sample_rate / 10;
        err = snd_pcm_hw_params_set_buffer_size_near(m_pcm_handle, params,
                                                     &buf_frames);
        if (err < 0)
        {
            std::cerr << "无法设置缓冲区大小: " << snd_strerror(err) << std::endl;
            return false;
        }

        /* 7. 应用参数 */
        err = snd_pcm_hw_params(m_pcm_handle, params);
        if (err < 0)
        {
            std::cerr << "无法应用硬件参数: " << snd_strerror(err) << std::endl;
            return false;
        }

        std::cout << "音频参数已设置: " << m_sample_rate << " Hz, "
                  << m_channels << " ch"
                  << (m_downmix ? " (软件 6→2 降混)" : " (硬件直出)") << std::endl;
        return true;
    }
    // 获取一个采样字节数
    int AlsaPlayback::GetBytesPerSample() const
    {
        switch (m_format)
        {
        case SND_PCM_FORMAT_S8:
            return 1; // 8位有符号
        case SND_PCM_FORMAT_U8:
            return 1; // 8位无符号
        case SND_PCM_FORMAT_S16_LE:
            return 2; // 16位有符号小端
        case SND_PCM_FORMAT_S16_BE:
            return 2; // 16位有符号大端
        case SND_PCM_FORMAT_U16_LE:
            return 2; // 16位无符号小端
        case SND_PCM_FORMAT_U16_BE:
            return 2; // 16位无符号大端
        case SND_PCM_FORMAT_S24_LE:
            return 3; // 24位有符号小端
        case SND_PCM_FORMAT_S24_BE:
            return 3; // 24位有符号大端
        case SND_PCM_FORMAT_U24_LE:
            return 3; // 24位无符号小端
        case SND_PCM_FORMAT_U24_BE:
            return 3; // 24位无符号大端
        case SND_PCM_FORMAT_S32_LE:
            return 4; // 32位有符号小端
        case SND_PCM_FORMAT_S32_BE:
            return 4; // 32位有符号大端
        case SND_PCM_FORMAT_U32_LE:
            return 4; // 32位无符号小端
        case SND_PCM_FORMAT_U32_BE:
            return 4; // 32位无符号大端
        case SND_PCM_FORMAT_FLOAT_LE:
            return 4; // 32位浮点小端
        case SND_PCM_FORMAT_FLOAT_BE:
            return 4; // 32位浮点大端
        case SND_PCM_FORMAT_FLOAT64_LE:
            return 8; // 64位浮点小端
        case SND_PCM_FORMAT_FLOAT64_BE:
            return 8; // 64位浮点大端
        default:
            std::cerr << "不支持的音频格式" << std::endl;
            return 2; // 默认返回2字节
        }
    }
    // 获取采样格式
    snd_pcm_format_t AlsaPlayback::GetFormat() const
    {
        return m_format;
    }
    // 设置采样格式
    bool AlsaPlayback::SetFormat(snd_pcm_format_t format)
    {
        if (nullptr != m_pcm_handle)
        {
            // 设备已经打开
            std::cerr << "设备已经打开" << std::endl;
            return false;
        }
        m_format = format;
        return true;
    }
    bool AlsaPlayback::Prepare()
    {
        int ret = snd_pcm_prepare(m_pcm_handle);
        if(ret < 0)
        {
            return false;
        }
        return true;
    }
}