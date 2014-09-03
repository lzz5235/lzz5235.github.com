---
layout: post
title: "Install Debian7.6 in server"
categories: others

tags: debian
---
在塔式服务器中安装Debian 7.6
=============================
今天负责给实验室的server安装debian 7.6遇到很多问题，特在此小记一下。

由于塔式服务器不同于普通的pc，主板结构不同于pc的主板。

这个主板的特殊之处在于主板有两个cpu插槽，cpu是Intel Xeon E5530 4核八线程，如果插上两个cpu的话，可以拓展成为8核16线程。

![](/assets/pic/server1.png)

另外由于是塔式服务器，有众多的SATA接口，这台服务器就插了5个硬盘，总容量在1T。

开始我把debian装在距离master硬盘很远的一个位置，安装完成后总是进入grub rescue模式，说找不到/boot

然后我在rescue模式下列出了每一块硬盘挨个去找，找到了/boot，但是就是每次重启无法自动启动。后来发现了原来是bios的问题。

###因为bios有的最多只能识别130Gb的大小，之后的就望尘莫及了。所以我们需要把linux的 /boot 与 /分区安装在前135GB的区域

安装完成之后，进入debian系统。

新的问题来了：

由于server使用的是闭源net driver.所以这时，我们无法使用任何网络软件库，apt-get……

我们先从别的机子上下好了drivers，然后拷到server上。

然后
<pre><code>
#vim /etc/resolve.conf //设置dns 
 
#vim /etc/network/interfaces #编辑网网卡配置文件
auto lo
auto eth0 #开机自动连接网络
iface lo inet loopback
allow-hotplug eth0
iface eth0 inet static #static表示使用固定ip，dhcp表述使用动态ip
address 192.168.21.166 #设置ip地址
netmask 255.255.255.0 #设置子网掩码
gateway192.168.21.2 #设置网关
 
#ifconfig eth0 up
#/etc/init.d/networking restart
</code></pre>




这时server的网络才开启，然后通过网络仓库更新软件。

然后远程配置各种ssh git maillist服务…..etc…
经过这一次重装，我对server有了新的认识，他确实不同于一般的pc，对于数据与性能有着更高要求。
