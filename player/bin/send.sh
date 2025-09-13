#!/bin/bash

# 提示用户输入文件名
echo "请输入文件名"
read filename

# 设置目标 IP
ip="192.168.100.102"

# 使用 SCP 命令上传文件
# 注意：这里假设您有 root 用户的 SSH 密钥设置，否则需要密码
scp "$filename" "root@$ip:/home/root/bin/"
