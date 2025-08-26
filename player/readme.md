这里是扳级程序设计
主要：
1.对于开发板led，button，音频等驱动
2.网络变成连接server，接受server的控制

# 板级程序设计
这里由于自己的驱动了解的还是非常浅薄，所以主要针对应用层进行编码
硬件平台采用imx6ull，其他可以运行linux的平台皆可，但是需要区验证音频（扬声器的驱动）等外设的驱动验证，这里我采用的很多的资料都是基于正点原子的驱动，应用，Qt资料，如果有需要可以自己在docs目录下自取

后面不论是在这一个模块还是其台模块都会分步骤构建整个项目，由于项目整体比较简单，疏漏之处见谅

下面就是基于硬件平台（imx6ull）的一些驱动程序验证

## LED验证

本章内容参考《正点原子imx6ull Linuxc应用编程》
这里呢需要设计到文件系统的一些知识，文件系统知识我自己了解的也是很少，需要大家下去自己多了解，在linux操作系统下的一个概念就是一切皆文件，所以学习linux还是从事和linux相关的职业都绕不开linux内核和文件系统两条路的，最近我自己也在学习这一部分的知识，只是自己学习的很浅薄在这里比方便进行一些没必要的讲解，所以靠自觉啦

*这里关于linux下面外设的设备结点和挂载驱动这方面知识我自己学习的较少，如果大家发现错误还请见谅，及时指出我会更正*

*sysfs文件系统*
sysfs 是一个基于内存的文件系统，同 devfs、proc 文件系统一样，称为虚拟文件系统；它的作用是将内核信息以文件的方式提供给应用层使用
在linux应用层编程中想要控制硬件可以通过`/dev/`下面的目录结点,或者`/sys`目录下面的设备属性文件
在正点原子提供的linux内核和系统镜像中，led可以通过`/sys/class`进行访问

路径
`/sys/class/leds/sys-led`
ls 可以看到sys-led这个就是系统led了，可以通过文件的方式访问硬件资源
进入sys-led目录 ls就可以看到下面的一些关于led的控制方式亮度、电源、触发方式等等
`brightness  device  max_brightness  power  subsystem  trigger  uevent`

shell 方式
```shell
# 点亮led “>” 重定向符号
echo 1 > brightness
# 熄灭led
echo 0 > brightness
```

c语言编程方式
c语言编程方式无疑就是通过文件方式来访问了，通过打开led设备对应的描述文件，通过向文件里面输入不同的内容就可以控制硬件行为
c语言编程方式可见详细源文件
至此控制led已经完成了


## 音频验证

本章内容参考《正点原子imx6ull Linuxc应用编程》
这里是整个项目里面最重要的一点，音频验证涉及到整个项目的逻辑
音频这里就介绍如何录音播放等操作
[参考博客](https://blog.csdn.net/Xinbaibaiya12138/article/details/149325472?spm=1001.2101.3001.6650.2&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EYuanLiJiHua%7EPosition-2-149325472-blog-133771032.235%5Ev43%5Epc_blog_bottom_relevance_base7&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EYuanLiJiHua%7EPosition-2-149325472-blog-133771032.235%5Ev43%5Epc_blog_bottom_relevance_base7&utm_relevant_index=5)

### ALSA框架概述

ALSA框架是linux下面的高级音频开发子系统，也可以称作为框架。现在Linux系统下的主流音频开发框架就是`ALSA`，ALSA替代了`OSS开发声音系统`，从概述来看ALSA框架我们可以分为三层，从底层到内核空间再到应用空间，分别对应不同的学习体系，例如硬件下需要去学习驱动等，在应用层的话需要熟练使用`alsa-lib`提供的库函数，作为我们编程的基础

![image-20250825230326866](C:\Users\欧鑫\AppData\Roaming\Typora\typora-user-images\image-20250825230326866.png)

在Linux庞大的体系中，除了针对音频开发的`ALSA`框架还需要学习其它框架，例如视频框架`V4L2`和	`ffmpeg`以及音频框架下的`Pipewire`，学习Linux开发，体系过于庞大，需要我们去学习的东西还有很多



### alsa-lib介绍

在我们学习应用层编程最绕不开的就是怎么调用库函数`API`...当然`alsa-lib`就是`ALSA`框架提供的一组编程接口，学习编程接口不需要我们过多的去硬背API的个个参数，理解性学习即可，更多还是要我们去学习框架的概念，架构，培养我们的学习方式

`alsa-lib`是linux提供的一组应用层的c库函数，为音频应用开发提供了一套标准的接口，应用层无需关注底层驱动等实现，只需要我们通过库函数即可实现对硬件设备声卡的驱动实现播放录音等操作

[alsa-lib官方文档地址](https://www.alsa-project.org/alsa-doc/alsa-lib/)

当然啦，如果英文过于晦涩难懂丢给AI也是一份不错的差事，让它生成对应的API参数说明和编程示例效率反而会更高





### sound设备结点

在 `Linux` 内核设备驱动层、基于 `ALSA `音频驱动框架注册的 `sound `设备会在`/dev/snd `目录下生成相应的 设备节点文件，cd到linux的dev目录下，ls即可看到sound对应的设备文件，同理在我们的硬件平台`imx6ull`的shell下也可以查询到对应的设备结点文件

在设备文件夹里面主要由如下内容：

- controlC0**：**用于声卡控制的设备节点，譬如通道选择、混音器、麦克风的控制等，C0 表示声卡 0 （card0）；  
- pcmC0D0c**：**用于录音的 PCM 设备节点。其中 C0 表示 card0，也就是声卡 0；而 D0 表示 device  0，也就是设备 0；最后一个字母 c 是 capture 的缩写，表示录音；所以 pcmC0D0c 便是系统的声卡 0 中的录音设备 0；  
- pcmC0D0p**：**用于播放（或叫放音、回放）的 PCM 设备节点。其中 C0 表示 card0，也就是声卡 0； 而 D0 表示 device 0，也就是设备 0；最后一个字母 p 是 playback 的缩写，表示播放；所以 pcmC0D0p 便是系统的声卡 0 中的播放设备 0；  
- pcmC0D1c**：**用于录音的 PCM 设备节点。对应系统的声卡 0 中的录音设备 1； 
- pcmC0D1p**：**用于播放的 PCM 设备节点。对应系统的声卡 0 中的播放设备 1；
- timer**：**定时器

其中在我的ubuntu下面特有一个seq文件，seq是ALSA的序列器，可以为应用程序之间提供基于时间和序列的高精度MIDI（Musical Instrument Digital Interface - 音乐设备数字接口）保证数据和事件的路由与调度

我们操作Linux下面的任何硬件设备都是通过操作文件的方式来控制（因为在Linux系统下一切设备皆为文件的概念），所以在应用层抽象成文件io的方式来操作设备结点里面的文件让我们无需去关注底层的驱动实现即可操作硬件

在linux系统下的`/proc/asound`目录下面，存在很多文件，它们描述了系统的声卡等信息

​	**cards**文件

cards文件里面描述了系统中可用的、注册的声卡



​	**devices**文件

devices文件描述了系统中所有声卡注册的设备，包括 control、pcm、timer、seq 等等



​	**PCM**

pcm文件描述了系统可用的pcm设备

```
在linux系统下面可以使用cat来查看这些设备信息
```



### alsa工具集使用



​	**aplay工具**

aplay工具用来播放音频等操作，这里可以类比ffmpeg里面的`ffplay`操作同样都是播放音频；aplay可以播放wav格式文件，不支持MP3格式文件，而ffmpeg工具中的ffplay就强大很多

​	**alsamixer工具**

用来配置声卡信息的工具，例如音量等

​	**alsactl工具**

用来保存声卡的配置信息，因为我们配置完毕声卡信息之后不保存，重启后声卡又恢复默认配置了

​	**amixer**

也是一个声卡信息配置工具，与alsamixer的区别是它是一个字符型的配置工具

​	**arecord**

是一个用于录音测试的工具

```c
arecord -f cd -d 10 test.wav // 录音十秒
```





至此alsa框架的简单介绍完毕了，后面会采用c语言的方式来学习和使用asla-lib，在学习过程中可能仅会针对自己在整个项目中需要使用到的一些函数去使用和学习，更多的内容还是需要自己去拓展吖



### alsa-libc编程





