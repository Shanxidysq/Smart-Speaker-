// 使用进程exec函数替换，使用aplay播放音频操作

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <sys/wait.h>
// main函数参数用来接收进程替换的音乐地址
int main(int argc,char* argv[])
{
    if(argc < 2)
    {
        fprintf(stderr,"main params invaild\n");
        exit(-1);
    }

    // 成功打开进程fork 然后进程替换
    pid_t pid = fork();
    // fork error
    if(pid < 0)
    {
        fprintf(stderr,"fork error\n");
        exit(1);
    }
    // 子进程 
    else if(pid == 0)
    {
        // 进行进程替换
        // argv[1] 播放的音乐地址
        execl("/usr/bin/aplay","aplay",argv[1],NULL);
        fprintf(stderr,"exec error\n");
    }
    // 父进程中进行控制
    // status位掩码获取子进程退出的状态
    int status;
    waitpid(pid,&status,0);
    exit(0);
}