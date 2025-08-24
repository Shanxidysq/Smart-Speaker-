# Smart-Speaker-
嵌入式linux项目-智能音箱

笔者这里学习开源项目-智能音箱（嵌入式linux应用层）
后续项目的搭建，开发学习，均通过github仓库同步



## 项目架构介绍



这里拆分整个项目架构，项目采用三个程序运行，分别为再板级程序运行的player（后面会统称为板级程序），在linux中运行的server，以及Qt设计的ui界面后面简称app

三个不同程序的交互主要还是采用socket网络编程，这里对网络编程的概念不会进行赘述，app和server主要对板级程序进行控制，server在linux上运行，因为qt的可移植性所以qt在linux和windows系统上运行均可；最后也是最重要的板级程序了。板级程序基于硬件平台imx6ull，配套开发板型号：**正点原子的阿尔法开发板**

板级程序做的事情就很多了，比如对硬件外设的驱动（LED灯，扬声器的输出，按键等），基于网络编程环境和server进行通信


上面赘述了简单的项目架构和硬件平台，在项目的每一个文件夹里面均有一个readme，里面会仔细描述我后续如何实现整个项目

## 开发环境搭建

在进行嵌入式linux开发的时候，我们通常需要一些辅助工具来进行开发，例如交叉编译工具链，FTP服务等等，在这里就简单介绍以下需要的工具及配置

### FTP文件传输服务配置

在ubuntu下启用FTP服务

```c
// 安装vsftpd
sudo apt-get install vsftpd 
// 安装完成之后 vim打开vsftpd配置文件
sudo vi /etc/vsftpd.conf
// 设置下面参数使能
local_enable=YES 
write_enable=YES 
// 重启FTP服务
sudo /etc/init.d/vsftpd restart 
```
到这里整个ubuntu环境下的FTP服务就配置完成了，后面只需要我们在win环境下安装客户端即可

在win下配置FTP客户端
在win下常用的客户端配置就是`FileZilla`了，这里我似乎忘记了需不需要魔法来配置，有需要这里可以自己百度一下，FTP的客户端后续使用就很方便了，输入ubuntu的ip地址，然后用户名和密码就可以进行有效的文件传输，在这里如果使用vaware还可以直接拖拽的方式

对了，很重要的一点就是，使用FTP服务可以设置ubuntu的ip地址设置成为静态ip这样不论是在后面的操作上还是使用FTP服务都很方便

### ssh服务

ssh服务大家应该都不陌生了，在vscode IDE下可以通过ssh链接ubuntu这样很方便，还有就是涉及到代码仓库等等操作
```c
// 开机自启并立即启动 查看运行状态（active 即正常）
sudo apt update
sudo apt install openssh-server
sudo systemctl enable --now ssh      
sudo systemctl status ssh       
// 放通防火墙
sudo ufw allow ssh   
// 生成密钥
ssh-keygen -t rsa -C "邮箱号"

// 生成了密钥之后在家目录下面有一个.ssh文件夹，把pub公钥添加到远程仓库这样管理代码就会很方便
```



### 交叉编译工具链

通常我们使用的系统都是x86_64架构的，而编译出来的程序想要在ARM架构下的开发板运行，就必须通过交叉编译工具链来编译，然后将编译出来的可执行程序移植到开发板当中

这里我使用的交叉编译工具是正点原子在开发资料包中提供的工具
`gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf.tar.xz`,这里并没有对其它交叉编译工具进行一个过多的了解

交叉编译工具链传输完毕之后
```c
// 在 /usr/local/ 目录下面创建arm文件夹用来存放交叉编译工具链 
sudo mkdir /usr/local/arm 
// 拷贝工具链到arm文件夹
sudo cp gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf.tar.xz /usr/local/arm/ -f 
// 使用tar -vxf命令加压交叉编译工具
sudo tar -vxf gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf.tar.xz 
// 修改环境变量
sudo vi /etc/profile 
// 在profile文件最后一行添加如下
export PATH=$PATH:/usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin 
// 安装库文件
sudo apt-get install lsb-core lib32stdc++6 
// 验证 可以看到版本号就是安装成功了 如果不成功可以重启ubuntu
arm-linux-gnueabihf-gcc -v 

```
至此我们的交叉编译工具链就已经安装完成了

至此我们在开发中需要的大部分软件已经安装完成了，后续要需要安装SecureCRT或者putty等软件我也就不在此赘述了大家可以参考其它魔法教程来操作


## 项目构建工具

对于脱离的集成式的IED之后项目构建成为了一个让人很头疼的问题，这里主要采用makefile作为项目构建工具当然了，如果大家会Cmake一样的
然后就是项目编译完毕了怎么移植到开发板呢可以只用scp命令
scp命令示例
```c
scp local_file.txt username@remote_host:/remote/directory/

例如
scp led username@ip:/root/work
```

