---
layout: post
title: "kdump 在Debian与Fedora下的配置"
categories: linux
tags: Debian,Fedora
---
kdump 在Debian与Fedora下的配置
================================
0.why we use kdump?

Kdump is a kernel crash dumping mechanism and is very reliable because the crash dump is captured from the context of a freshly booted kernel and not from the context of the crashed kernel. Kdump uses kexec to boot into a second kernel whenever system crashes. This second kernel, often called the crash kernel, boots with very little memory and captures the dump image.

The first kernel reserves a section of memory that the second kernel uses to boot. Kexec enables booting the capture kernel without going through the BIOS, so contents of the first kernel’s memory are preserved, which is essentially the kernel crash dump.

使用mce-inject注入fatal错误的时候，system会直接panic。不像UCNA，SRAO等可以通过mcelog，rasDamon查看。
system在进去machine check 后，内核调用，用户态下的mcelog，rasDamon都已经无法使用，导致dmesg信息MCE解码信息都无法查看。

kdump原理就是在dump kernel panic后，重新启动后利用kexec载入debug kernel 然后利用这个内核去dump出现panic错误的内核信息。

然后system使用正常kernel启动后，可以分析该dump文件，包括查看demsg/memory/PCIe等各种kernel信息等。

用来诊断kernel出现了哪些致命错误。
在本项目中主要利用该工具用来分析注入mce 错误产生的dumpcore信息。

————————————————————————–

首先我们分清要使用的系统：

###Fedora Configure


由于Fedora的软件比较新，如果你用的是是Fedora 20

我们直接可以 

yum install –enablerepo=fedora-debuginfo –enablerepo=updates-debuginfo kexec-tools crash kernel-debuginfo

下载安装好kdump crash kexec工具

2.然后我们在/etc/default/grub里面的

GRUB_CMDLINE_LINUX=”rd.lvm.lv=fedora/swap rd.md=0 rd.dm=0 $([ -x /usr/sbin/rhcrashkernel-param ] && /usr/sbin/rhcrashkerne l-param || :) rd.luks=0 vconsole.keymap=us rd.lvm.lv=fedora/root rhgb quiet crashkernel=240M”
最后加入crashkernel=240M

这个预先留出的内存大小，在下次使用就被占用。
一般 1G——80M 2G——-160M 3G——–240M

最后更新grub ：$update-grub2 即可

3.重新启动，选择该内核进入system

<pre><code>
[root@localhost mce-inject-encode]# service kdump start
Redirecting to /bin/systemctl start kdump.service
[root@localhost mce-inject-encode]# systemctl status kdump
kdump.service - Crash recovery kernel arming
Loaded: loaded (/usr/lib/systemd/system/kdump.service; enabled)
Active: active (exited) since 一 2014-10-27 09:15:31 CST; 2 days ago
Process: 652 ExecStart=/usr/bin/kdumpctl start (code=exited, status=0/SUCCESS)
Main PID: 652 (code=exited, status=0/SUCCESS)
CGroup: /system.slice/kdump.service
 
10月 27 09:15:22 localhost.localdomain systemd[1]: Starting Crash recovery kernel arming...
10月 27 09:15:31 localhost.localdomain kdumpctl[652]: kexec: loaded kdump kernel
10月 27 09:15:31 localhost.localdomain kdumpctl[652]: Starting kdump: [OK]
10月 27 09:15:31 localhost.localdomain systemd[1]: Started Crash recovery kernel arming.
10月 29 09:29:36 localhost.localdomain systemd[1]: Started Crash recovery kernel arming.
</code></pre>

4.开始等待panic错误，如果发现panic错误，system会立刻切换到crashkernel收集信息，收集完毕重新启动。

5.进入到system。键入到/var/crash文件夹下有通过kdump产生的文件：

<pre><code>
[root@localhost 127.0.0.1-2014.10.29-09:59:07]# ls
vmcore vmcore-dmesg.txt
[root@localhost 127.0.0.1-2014.10.29-09:59:07]# vim vmcore-dmesg.txt
 
[175451.136453] Triggering MCE exception on CPU 0
[175451.136463] [Hardware Error]: MC0 Error:
[175451.136467] [Hardware Error]: Corrupted MC0 MCE info?
[175451.136469] [Hardware Error]: Error Status: System Fatal error.
[175451.136472] [Hardware Error]: CPU:0 (f:6b:2) MC0_STATUS[-|UE|-|PCC|-]: 0xb200000000000000
[175451.136475] [Hardware Error]: cache level: RESV, tx: INSN
[175451.136480] mce: [Hardware Error]: CPU 0: Machine Check Exception: 0 Bank 0: b200000000000000
[175451.136662] mce: [Hardware Error]: TSC 15f2d1714af92
[175451.136765] mce: [Hardware Error]: PROCESSOR 2:60fb2 TIME 1414547938 SOCKET 0 APIC 0 microcode 0
[175451.136933] [Hardware Error]: MC0 Error:
[175451.137020] [Hardware Error]: Corrupted MC0 MCE info?
[175451.137132] [Hardware Error]: Error Status: System Fatal error.
[175451.137249] [Hardware Error]: CPU:0 (f:6b:2) MC0_STATUS[-|UE|-|PCC|-]: 0xb200000000000000
[175451.137412] [Hardware Error]: cache level: RESV, tx: INSN
[175451.137431] mce: [Hardware Error]: Machine check: Invalid
[175451.137431] Kernel panic - not syncing: Fatal machine check on current CPU
</code></pre>

通过vmcore-dmesg.txt 可以查看panic时的dmesg 。如果需要对panic core 进行debug的时候，需要使用[root@localhost 127.0.0.1-2014.10.29-09:59:07]# crash vmcore /usr/lib/debug/lib/xxxxxx/vmlinux

PS:注意这个vmcore与你的debugcore 版本要一致否则无法进行crash！

创建kernel-debuginfo-devel

[https://fedoraproject.org/wiki/Building_a_custom_kernel](https://fedoraproject.org/wiki/Building_a_custom_kernel)

[https://fedoraproject.org/wiki/How_to_create_an_RPM_package/zh-cn](https://fedoraproject.org/wiki/How_to_create_an_RPM_package/zh-cn)

[http://blog.csdn.net/justlinux2010/article/details/9905425](http://blog.csdn.net/justlinux2010/article/details/9905425)

[http://www.linuxfly.org/post/130/](http://www.linuxfly.org/post/130/)

具体操作看 [dpkg/rpm](http://lzz5235.github.com/2014/12/10/rhelfedoracentos-rpm.html)

————————————

###Debian Configure


因为在本项目中我们使用kernel 3.14.8,所以我们要使用crash 7.0.8.在Deibian 7.6中，默认的crash 6.0.6版本中
无法分析kernel 3.6以及以后内核的SLAB内存，crash载入dump文件，会出现以下错误信息：

<pre><code>
please wait... (gathering kmem slab cache data)
crash: invalid structure member offset: kmem_cache_s_next
FILE: memory.c LINE: 7903 FUNCTION: kmem_cache_init()
[/usr/bin/crash] error trace: 466a99 =&gt; 47b44b =&gt; 491f5c =&gt; 547d43
547d43: OFFSET_verify+164
491f5c: (undetermined)
47b44b: vm_init+10841
466a99: main_loop+239
</code></pre>

一个方法是我们可以在/etc/apt/source.list 中将wheezy改为jessie，可以顺利将crash升级到7.0.8

Debian Configure 有一个问题是需要手动配置crashkernel 内核地址

1. aptitude install kdump-tools crash kexec-tools makedumpfile

2. 在 /etc/default/grub中修改为GRUB_CMDLINE_LINUX_DEFAULT=”crashkernel=128M quiet”

3. update-grub2

4. 里面的DEBUG_KERNEL也要对应产生的vmcore版本,进行crash时vmcore与这个debug_kernel must版本一致!

主要是在编译内核的目录下使用make deb-pkg 这时会产生专门针对3.14.8内核的dbg vmlinux，以及header

<pre><code>
root@debian:/usr/src/kernels# ls
linux-3.14.8 linux-headers-3.14.8_3.14.8-2_amd64.deb linux-libc-dev_3.14.8-2_amd64.deb
linux-3.14.8.tar linux-image-3.14.8_3.14.8-2_amd64.deb
linux-firmware-image-3.14.8_3.14.8-2_amd64.deb linux-image-3.14.8-dbg_3.14.8-2_amd64.deb
</code></pre>

然后安装这些生成的deb.

5.在/etc/default/kdump-tools中配置为

<pre><code>
USE_KDUMP=1
KDUMP_SYSCTL="kernel.panic_on_oops=1"
KDUMP_KERNEL=/boot/vmlinuz-3.14.8
KDUMP_INITRD=/boot/initrd.img-3.14.8
KDUMP_COREDIR="/var/crash"
KDUMP_FAIL_CMD="reboot -f"
DEBUG_KERNEL=/usr/lib/debug/boot/vmlinux-3.14.8
MAKEDUMP_ARGS="-c -d 31"
 
//在/etc/default/kexec中配置为：
root@debian:/usr/src/kernels# cat /etc/default/kexec
# Defaults for kexec initscript
# sourced by /etc/init.d/kexec and /etc/init.d/kexec-load
 
# Load a kexec kernel (true/false)
LOAD_KEXEC=true
 
# Kernel and initrd image
KERNEL_IMAGE="/boot/vmlinuz-3.14.8"
INITRD="/boot/initrd.img-3.14.8"
 
# If empty, use current /proc/cmdline
APPEND=""
 
# Load the default kernel from grub config (true/false)
USE_GRUB_CONFIG=false
</code></pre>

6.重新启动：

<pre><code>
root@debian:/usr/src/kernels# service kdump-tools restart
[ ok ] unloaded kdump kernel.
[ ok ] loaded kdump kernel.
</code></pre>

7.kdump就开始捕捉错误。

8.关于分析dump的操作crash

<pre><code>
This GDB was configured as "x86_64-unknown-linux-gnu"...
 
      KERNEL: kernel_link
    DUMPFILE: dump.201410311601  [PARTIAL DUMP]
        CPUS: 2
        DATE: Fri Oct 31 16:01:04 2014
      UPTIME: 00:09:57
LOAD AVERAGE: 0.21, 0.11, 0.07
       TASKS: 209
    NODENAME: debian
     RELEASE: 3.14.8
     VERSION: #2 SMP Fri Oct 31 14:38:20 CST 2014
     MACHINE: x86_64  (2199 Mhz)
      MEMORY: 1.7 GB
       PANIC: "Kernel panic - not syncing: Fatal machine check on current CPU"
         PID: 0
     COMMAND: "swapper/1"
        TASK: ffff88006d7ea8e0  (1 of 2)  [THREAD_INFO: ffff88006d7ee000]
         CPU: 1
       STATE: TASK_RUNNING (PANIC)
 
crash> help
 
*              files          mach           repeat         timer
alias          foreach        mod            runq           tree
ascii          fuser          mount          search         union
bt             gdb            net            set            vm
btop           help           p              sig            vtop
dev            ipcs           ps             struct         waitq
dis            irq            pte            swap           whatis
eval           kmem           ptob           sym            wr
exit           list           ptov           sys            q
extend         log            rd             task           
 
crash version: 7.0.8    gdb version: 7.6
For help on any command above, enter "help <command>".
For help on input options, enter "help input".
For help on output options, enter "help output".
 
crash>
</code></pre>

参考：

[http://wiki.incloudus.com/display/DOC/Debian+-+Kdump](http://wiki.incloudus.com/display/DOC/Debian+-+Kdump)

[http://fedoraproject.org/wiki/How_to_use_kdump_to_debug_kernel_crashes](http://fedoraproject.org/wiki/How_to_use_kdump_to_debug_kernel_crashes)

[http://debian-handbook.info/browse/stable/sect.kernel-compilation.html](http://debian-handbook.info/browse/stable/sect.kernel-compilation.html)
