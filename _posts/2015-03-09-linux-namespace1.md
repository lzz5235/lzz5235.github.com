---
layout: post
title: "Linux namespace分析（1）"
categories: linux
tags: namespace
---
在linux中实现kernel virtualization 的条件就是资源的限制与隔离，而namespace完成的系统资源的隔离。

从Linux 2.6.23开始 对于namespace的框架实现基本完成，之后的patch大多是修修补补，比较重要的一个patch是linux 3.8中一个非root用户可以创建user namespace，在这个user namespace中，该用户拥有所有的权限，包括在这个namespace中创建其他类型的namespace。

Linux namespace主要包含：

* UTS namespace

* Mount namespace

* IPC namespace

* PID namespace

* Network namespace

* User namespace

UTS namespace主要可以使得host拥有两套nodename与domainname：最大的区别就是如果使用lxc或者docker启动一个镜像，使用uname可以获得不同的主机名。

Mount namespace使得不同namespace中的相同进程都对文件结构有不同的视角，比起chroot有更好的安全性，使用namespace特性，可以使得系统的mount namespace产生主从的结构。

IPC namespace 使得每个IPC内的进程相互通信。

PID namespace最为神奇，他使得进程在不同的PID namespace中可以拥有相同的PID，也就是说如果系统拥有root namespace、namespace A、namespace B 。其中A、B是root的子命名空间，也就是说在A、B命名空间内部可以存在两个相同的PID，比如init 。当然这两个init的PID号从root命名空间来看是不同的，这里就有了映射的概念。

Network namespace 可以使得一个物理主机的网卡，模拟出两个虚拟网卡，每个虚拟网卡都可以绑定相同的端口，访问却不受影响。

User namespace 是在linux 3.8 才完成的部分，他可以使得一个进程的User ID和group ID相比于命名空间内外不同。举例：在root namespace中的一个非特权进程在namespace A中可以是init 0 ，可以是一个特权进程！

以上这些空间可以使得linux建立起一个轻量级的虚拟机系统。比如lxc、docker。

参考：

[http://lwn.net/Articles/531114/](http://lwn.net/Articles/531114/)
