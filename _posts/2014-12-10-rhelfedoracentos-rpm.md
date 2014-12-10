---
layout: post
title: "RHEL系(Fedora,CentOS) rpm打包"
categories: linux
tags: Fedora,CentOS
---
RHEL系(Fedora,CentOS) rpm打包
=============================
现在Linux下越来越倾向于打包二进制安装文件，rpm，deb都是这种类型。

但是作为内核开发人员我们要对程序进行调试，就要有debug-info package，但是默认内核人是不提供debug-info的包，所以需要我们自动生成。

一般我们要用root用户进行编译，但是编译rpm包，至少需要20GB的临时文件空间！
一键操作：make rpm ———–>自动身成rpmtree,kernel.spec文件，自动编译。

——————————

详细定制：

下面开始创建：

1.用所在的用户输入rpmdev-setuptree，然后会在该用户的目录下生成${HOME}/rpmbuild/SOURCES ，${HOME}/rpmbuild/SPECS 和 ${HOME}/rpmbuild/BUILD 。

2.在多数源玛目录下会自动带有kernel.spec的文件，将这个文件放到${HOME}/rpmbuild/SPECS中，将kernel-3.14.8.tar.gz 放入${HOME}/rpmbuild/SOURCES中，然后在SOURCE中放入.config,

默认生成的spec带着”%debug_package %{nil}” 这就意味着不会生成带debug_info的包。如果需要，我们就要注释掉。
cp .config ~/rpmbuild/SOURCES/config-`uname -m`-generic

3.切换到SPECS目录下。

建立所有内核配置：
rpmbuild -bb –target=`uname -m` kernel.spec

关闭指定的内核配置（为了更快的建立）：
rpmbuild -bb –without <option> –target=`uname -m` kernel.spec

其中“ option ”的有效值包括xen、smp、up、pae、kdump、debug 和 debuginfo。指定 –without debug 会剔除内核中的调试代码，指定 –without debuginfo 会禁止建立 kernel-debuginfo 包。

只建立一个特定的内核：

rpmbuild -bb –with <option> –target=`uname -m` kernel.spec
“ option ”的有效值包括 xenonly、 smponly 和 baseonly 。

例如，只建立 kernel 和 kernel-devel 包的命令是：

rpmbuild -bb –with baseonly –without debuginfo –target=`uname -m` kernel.spec
建立时包含固件，用如下命令：

rpmbuild -bb –with baseonly –with firmware –without debuginfo  –target=`uname -m` kernel.spec 

建立的过程需要很长时间。会在屏幕上打印大量的信息。这些信息可以被忽略，除非建立过程因为一个 error 而停止。如果成功完成建立过程，一个新的内核包会出现在 ~/rpmbuild/RPMS 目录。

 

参考：

[https://fedoraproject.org/wiki/Building_a_custom_kernel](https://fedoraproject.org/wiki/Building_a_custom_kernel)

[https://fedoraproject.org/wiki/How_to_create_an_RPM_package/zh-cn](https://fedoraproject.org/wiki/How_to_create_an_RPM_package/zh-cn)

[http://blog.csdn.net/justlinux2010/article/details/9905425](http://blog.csdn.net/justlinux2010/article/details/9905425)

[http://www.linuxfly.org/post/130/](http://www.linuxfly.org/post/130/)


