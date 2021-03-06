---
layout: post
title: "裁剪编译Linux Kernel"
categories: [linux]

tags: [Kernel]

---
裁剪编译Linux Kernel
=======================================================
参考[http://www.wikihow.com/Compile-the-Linux-Kernel]()

  最近由于项目需要，需要编译裁剪符合需求的kernel。大家都知道distribution的版本大都比较庞大，里面很多东西其实对于自己的机器根本用不到，在嵌入式系统中由于flash与rom的大小有限。显得裁剪kernel更加有必要。

自己编译内核能在选项里去除这些多余的选项， 只挑选当前机器适用的硬件驱动， 往往能得到1个更精简的内核，理论上是比1个臃肿的内核更加稳定和快速的。 第二，对于任何软件来讲。理论上用当前机器自己编译出来的会比用其他环境机器编译出来的性能要高一点点。

其实裁剪kernel很简单，我主要用Fedora做演示。

* 首先你需要有必要的工具集，比如 ncurses-devel  gcc g++ *aclocal kernel-devel等。使用yum 安装就好

* 去https://www.kernel.org/下载你要编译的代码

* 也许下载的是tar.xz格式的。我们可以先使用xz -d 后使用 tar -vxf。然后进入源码目录。

* 我们可以使用make menuconfig挨个配置硬件。包括cpu driver什么的。（重新编译使用make mrproper清除目标文件）

* 要明白显卡，声卡，cpu, 网卡（有线or无线） 的型号信息， 起码要知道芯片的型号。 因为这些驱动都必须在内核配置里手动制定的。 具体信息可以用 lspci 或者 lsusb 或者 dmesg 命令来查看。

* 编译的时候，系统是会检索”.config” 最这个文件作为配置文件的， 每次执行清理的时候会把这个文件清楚， 所以再编译前要把你保存的配置文件命名为.config。比如我再Fedora下面，我们可以把/boot/下面的config-XXXXX拷到源码文件夹下，并以.config命名。

* time make make all -j4  #红色部分是制定4个线程编译的意思。如果是双核cpu就设为2吧。其中time 是顺便计时的命令。

#####* fedora 编译完后并不生成安装包， 但是实际上内核文件和模块和头文件都编译出来了。只需要在当前源代码目录下执行：

#####make modules

#####make modules_install #这一步是安装模块

#####make install #安装内核和头文件。fedora执行安装命令完后系统也会自动帮你更新grub配置。

* 卸载旧内核

fedora 自编译并不是用包管理器安装的， 所以要手动删除内核和模块，并更新gurb设置，当然个人建议写成1个脚本，用root来执行就ok啦， 脚本参照下面：
rm -rf /lib/modules/XXXXX/ #删除模块
ls /boot/XXXXX | xargs rm #删除内核 注意有3个文件，注意关键字 这一步要小心
rm -rf /boot/grub2/grub.cfg #删除grub配置
grub2-mkconfig -o /boot/grub2/grub.cfg #重新生成grub配置

 

____最近编了定制了一些内核，遇到了一些问题。比如高版本gcc编译低版本内核，会出现 ata4 failed现象。这时我们要查看gcc版本，可以选择gcc 4.3版本。__

多次编译需要清理。

____比如使用make clean会清理掉除.config文件。make mrproper清楚所有temp文件包括.config文件。__

