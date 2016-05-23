---
layout: post
title: "在pcDuino上通过Cylon控制硬件"
categories: others 
tags: pcDuino,Cylon,nodejs
---
在pcDuino上通过Cylon控制硬件
===========================

##准备工作：
我们用的板子型号是pcDuino3B（http://www.linksprite.com/linksprite-pcduino3/）。默认系统是ubuntu 12.04。推荐从官网下载固件（http://www.linksprite.com/linksprite-pcduino3/）把pcDuino刷到buntu 14.04,因为nodejs发展迭代比较快，低版本的gcc，g++无法编译某些nodejs。

##安装具体的构建工具：

1. armhf的源非常少，所以例如sohu，163很多arm设备的源根本不支持，推荐原装的源，可能网速慢些，但起码比较齐全。
2. apt-get install build-essential
3. apt-get install git
4. ubuntu 12.04 的软件仓库中版本比较老，从仓库直接安装nodejs和npm后，安装grunt会存在问题，所以推荐源码安装，或者直接官网armv7二进制版本安装。
5. wget https://nodejs.org/dist/v6.1.0/node-v6.1.0-linux-armv7l.tar.xz
6. 解压缩node-v6.1.0-linux-armv7l.tar.xz
7. cd node-v5.10.1-linux-armv7l
8. 将其复制到/usr/local下即可使用
9. 查看node 版本 ：node -v
10. 查看npm版本：npm -v
11. 安装好npm后，设置国内npm源：npm config set registry https://registry.npm.taobao.org

在安装过程中，我遇到很多坑，比如安装node-inspector，最后返回 ELIFECYCLE，通过查找资料，我们可以使用–unsafe-perm，对于一些在armv7上没有编译的二进制包，我们可以使用–build-from-source，直接本地编译（尽量使用g++ 4.8之后的版本）。

##开始写代码：

比如我们在一个文件中写入：

<pre><code>
'use strict';
 
    console.log("hello test");
    var Cylon = require("cylon");
 
    Cylon.robot({
      connections: {
        pcduino: { adaptor: 'pcduino' }
      },
 
      devices: {
        led: { driver: 'led', pin: 10 }
      },
 
      work: function(my) {
       every((1).second(), my.led.toggle);
      }
    }).start();
</code></pre>
   
然后通过node运行，查看安装效果：

![](/assets/pic/20160517214206.jpg)

目前已有很多项目在使用这个serialport包进行串口处理：

* Johnny-Five – Firmata based Arduino Framework.

* Cylon.js – JavaScript Robotics, By Your Command.

* node-l8smartlight (source) A node library to control the L8 Smartlight via Bluetooth or USB port

* firmata Talk natively to Arduino using the firmata protocol.

* tmpad source – a DIY midi pad using infrared, arduino, and nodejs. Video

* duino – A higher level framework for working with Arduinos in node.js.

* Arduino Drinking Game Extravaganza – AKA “The Russian” a hexidecimal drinking game for geeks by Uxebu presented at JSConf EU 2011.
* Arduino controlling popcorn.js – Controlling a popcorn.js video with an Arduino kit.

* Robotic JavaScript – The first live presentation of the node-serialport code set as presented at JSConf EU 2010.

* devicestack – This module helps you to represent a device and its protocol.

* reflecta A communication protocol that combines Arduino Libraries and NodeJS into an integrated system.

* rc4pt-node – Control Popcorntime with an Infrared receiver and Arduino.

##参考

[1] [http://www.linuxidc.com/Linux/2016-04/130093.htmnodejs](http://www.linuxidc.com/Linux/2016-04/130093.htmnodejs)

[2] 官方代码仓库：[https://nodejs.org/download/release/latest/](https://nodejs.org/download/release/latest/)

[3] 配置淘宝npm源：[http://www.cnblogs.com/trying/p/4064518.html](http://www.cnblogs.com/trying/p/4064518.html)

[4] [http://cnodejs.org/topic/4f9904f9407edba21468f31e](http://cnodejs.org/topic/4f9904f9407edba21468f31e)
