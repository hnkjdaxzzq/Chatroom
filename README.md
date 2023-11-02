# Web Server

## Introduction
本项目为使用C++开发的Web服务器，解析了get请求和post请求，能够处理静态资源，支持HTTP长链接，实现了异步日志。

## Envoirment
- OS: Ubuntu 22.04
- Complier: clang-17.0.4
- Build tool: xmake

## Build
- 安装[xmake](https://xmake.io/#/zh-cn/guide/installation)
- 在当前目录下输入`xmake`即可自动编译，若要使用xmake其它的编译功能，请查看xmake官方文档。
- 编译完成的可执行文件在`Chatroom/build/linux/x86_64/debug`或者`Chatroom/build/linux/x86_64/release`下

## Usage
```sh
./server [-p port] [-d webDir] [-t threads_numbers] [-l logpath] [-v loglevel]
```
### example:
```sh
./server -p 8888 -d /zzq/home/myblog -t 6 -l /zzq/home/log -v 0
```

## Model
并发模型为Reactor+非阻塞IO+线程池。