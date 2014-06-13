---
layout: post
title: "cscope ctags vim 使用"
categories: others

tags: [vim,C/C++]
---
cscope ctags vim 使用
=====================
最近终于忙完了导师组织的Google devfest 的活动，有了自己学习的时间，因为我加入的项目组的缘故，所以需要看一些kernel的代码，大家都知道kernel的代码比较复杂混乱，很多functions经常会跳来跳去，最后跳的我们都不知道这个值是怎么来的了。对于大型的C/C++工程更是如此，不同的source code 分布在不同的文件夹，靠我们自己打开很繁琐，cscope配合vim查看源代码就是不错的选择。
至于cscope与ctags的安装，我就不说了，使用localinstall 或者apt-get都很方便。
比如用我的ChatOnline项目为例，我先将工作目录切换到code的顶层目录，然后使用

<pre><code>
[root@localhost Server]# cscope -Rbq
</code></pre>
含义是：
-R: 在生成索引文件时，搜索子目录树中的代码，因为我们已经在cscope.files中把所有的文件都打印入了列表中，所以
-b: 只生成索引文件，不进入cscope的界面
-q: 生成cscope.in.out和cscope.po.out文件，加快cscope的索引速度

![](/assets/pic/2013-10-24-195819.png)

ctags系统自带，先说说taglist吧（很简单，很小，才49K）：

1. taglist
能够列出源文件中的tag(function, class, variable, etc)并跳转.
注意：taglist依赖于ctags，所以要先装ctags，否则taglist装了也没法用！
（1）到http://vim.sourceforge.net/scripts/script.php?script_id=273
下载最新的taglist,我的版本是taglist_45.zip，解压你会发现一个文件夹：plugin.
(2) 把上面文件夹放到自己的~/.vim文件夹里面，如果自己的家目录(主目录)没有这个隐藏的文件夹，就自己mkdir .vim一下，建立一个。
然后把doc和plugin拷到.vim文件夹里。或者直接考到/etc/vimrc中
然后我们输入

<pre><code>
[root@localhost Server]# ctags -R
</code></pre>
就会出现一个ctag的文件。

另外我们要配置vimrc才能使用快捷键来唤出taglist，如下图：
![](/assets/pic/2013-10-24-201308.png)

配置文件内容为：

<pre><code>
"""""""""""""""""Taglist设置"""""""""""""""""
map  : Tlist  ""按下F3就可以呼出了
"let Tlist_Auto_Open = 1  "在启动VIM后，自动打开taglist窗口
let Tlist_Ctags_Cmd = '/usr/bin/ctags'  "设定ctags的位置
let Tlist_Use_Right_Window=0 " 1为让窗口显示在右边，0为显示在左边
let Tlist_Show_One_File=0 "让taglist可以同时展示多个文件的函数列表，设置为1时不同时显示>多个文件的tag，只显示当前文件的
let Tlist_File_Fold_Auto_Close=1 "同时显示多个文件中的tag时，taglist只显示当前文件tag，>其他文件的函数列表折叠隐藏
let Tlist_Exit_OnlyWindow=1 "当taglist是最后一个分割窗口时，自动退出vim
"let Tlist_Use_SingleClick= 1    " 缺省情况下，在双击一个tag时，才会跳到该tag定义的位置
"let Tlist_Process_File_Always=0  "是否一直处理tags.1:处理;0:不处理
 
"""""""""""""""""""""""cscope设置""""""""""""""""""
set cscopequickfix=s-,c-,d-,i-,t-,e-
if has("cscope")
set csprg=/usr/bin/cscope
set csto=1
set cst
set nocsverb
" add any database in current directory
if filereadable("cscope.out")
   cs add cscope.out
endif
set csverb
endif
 
nmap <C-@>s :cs find s =expand("")
nmap <C-@>g :cs find g =expand("")
nmap <C-@>c :cs find c =expand("")
nmap <C-@>t :cs find t =expand("")
nmap <C-@>e :cs find e =expand("")
nmap <C-@>f :cs find f =expand("")
nmap <C-@>i :cs find i ^=expand("")$
nmap <C-@>d :cs find d =expand("")
</code></pre>
这样我们就可以使用F3唤出taglist。开始使用
ctl-w＋w或ctl-w＋ 方向键窗口切换（taglist本质上是一个vim分隔窗口，因此可以使用ctl-w系列快捷键对窗口进行切换操作)。

e:Find this egrep pattern//查找egrep模式，相当于egrep功能，但查找速度快多了
f:Find this file //查找并打开文件，类似vim的find功能
g:Find this definition//查找函数、宏、枚举等定义的位置，类似ctags的功能

有的时候按g会产生多个调用，那么我们要选择是在哪个文件哪个函数。
更多的cscope快捷键可以在vim 下 ：cs help

![](/assets/pic/2013-10-24-2026.png)


> ctrl+] 转到函数定义的地方

>ctrl+t 回退到函数调用的地方