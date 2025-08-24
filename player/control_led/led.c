// linux编程一些必要接口头文件
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// 宏定义的方式声明LED的操作路径
#define LED_TRIGGER "/sys/class/leds/sys-led/trigger"
#define LED_BRIGHTNESS "/sys/class/leds/sys-led/brightness"
// 宏函数 emm宏定义这里建议大家使用（）括起来
#define ERROR() ( fprintf(stderr, "main param error\n") )

// main函数的参数形式可以去了解一下
int main(int argc, char *argv[])
{
    // 对main函数的参数进行有效性检查
    if (1 > argc)
    {
        ERROR();
        exit(-1);
    }

    // 打开亮度控制文件
    int brightness_fd = open(LED_BRIGHTNESS, O_RDWR);
    if (-1 == brightness_fd)
    {
        fprintf(stderr, "open brightness error\n");
        exit(-1);
    }

    int trigger_fd = open(LED_TRIGGER, O_RDWR);
    if (-1 == trigger_fd)
    {
        fprintf(stderr, "open trigger error\n");
        exit(-1);
    }
    /* 设置触发模式也是一样的方式 */
    /* 根据传参控制 LED */
    if (!strcmp(argv[1], "on"))
    {
        write(trigger_fd, "none", 4); // 先将触发模式设置为 none
        write(brightness_fd, "1", 1);    // 点亮 LED
    }
    else if (!strcmp(argv[1], "off"))
    {
        write(trigger_fd, "none", 4); // 先将触发模式设置为 none
        write(brightness_fd, "0", 1);    // LED 灭
    }
    return 0;
    
}
