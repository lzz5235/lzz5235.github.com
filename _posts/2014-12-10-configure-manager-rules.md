---
layout: post
title: "Configure Manager Rules"
categories: others
tags: CM
---
Configure Manager Rules
========================
CM的职责就是对与git 进行管理，将不合格的commit拒绝在仓库以外。

详细开发流程
但是每周CM的职责除了合并开发成员的branch外，还要进行commit的汇报，比如commit多少，merge多少，reject多少等等。

比如我们将个git log >/tmp/git.log文件中，然后进行分析。

比如我们统计git log 中的commit 信息 可以使用 grep commit git.log | wc -l 方式计算commit行数。

<pre><code>
$grep commit git.log | wc -l
$grep Merge git.log | wc -l
</code></pre>

因为git log 的数据非常整齐，可以使用vim的块操作

输入v进入visual模式，然后可以移动光标选取一段文本。

d———->剪贴

y———->复制

p———->粘贴

对于经常登陆的机子我们可以生成dsa

> use ”ssh-keygen -t dsa” to generate a key

> then use ”ssh-copy-id /home/lzz/.ssh/id_dsa.pub user@server IP”

> Ok, now you can ssh and git pull/push without password

对于一些system如果ssh-copy-id 无法使用，我们可以手动将/home/lzz/.ssh/id_dsa.pub 手动拷贝到目标机器的目标用户目录下，
比如/home/user/.ssh/id_dsa_pub.A. 然后将这个id_dsa_pub.A >> authority 后面。

这些是CM的日常工作，除了这些，对于master branch与 local branch的backup也非常重要。
比如我有两台机子都要备份，我们可以把资料相互备份然后传输到对方的机子上。我们知道两台主机数据同时损坏的概率非常下。

对于每台主机，CM应该记录到以下几点：

<pre><code>
security incidents
power-fail
server off-line
filesystem overruns (filesystem full)
dmesg warnings/errors
abnormal operational behavior
</code></pre>

[backup_policy](/assets/resource/backup_policy.txt)

[cm_rules](/assets/resource/cm_rules.txt)

对于一些备份的例行任务，我们可以使用crontab -e 命令创建定时任务，关于这个网上的使用比较多，我们可以参考

[http://www.cnblogs.com/peida/archive/2013/01/08/2850483.html](http://www.cnblogs.com/peida/archive/2013/01/08/2850483.html)
