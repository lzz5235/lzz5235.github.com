---
layout: post
title: "Coccinelle 使用"
categories: linux
tags: Code,static
---
Coccinelle 使用
=============

Coccinelle是一个程序的匹配和转换引擎，它提供了语言SMPL（语义补丁语言）用于指定C代码所需的匹配和转换。Coccinelle 最初是用来帮助Linux的演变，支持更改库应用程序编程接口，比如重命名一个函数，增加一个依赖于上下文的函数参数或者重新组织一个数据结构。除此之外，Coccinelle页被人用来查找或者修复系统代码的bug。

项目地址：[https://github.com/coccinelle/coccinelle](https://github.com/coccinelle/coccinelle)

安装在这里不再赘述,这里要注意的是需要安装python的devel包，否则这个程序无法运行！

<pre><code>	
$git clone https://github.com/coccinelle/coccinelle
$git tag > git checkout -b build coccinelle-1.0.0-rc21
$apt-get install python2.6-dev libpycaml-ocaml-dev libmenhir-ocaml-dev menhir ocaml-native-compilers \
ocamlduce camlp4-extra ocaml-findlib pkg-config texlive-fonts-extra
$./configure --with-python --with-menhir
$make all
$apt-get remove coccinelle (prevent conflict)
$make install
</code></pre>

安装完毕之后，我们可以定义脚本

<pre><code>		
@search@
identifier fn,call;
statement s1,s2;
expression E1,E2;
int fd;
position p;
constant C;
@@
 
<+...
* fd=open@p(...);
//  ...when != fn(<+...fd...+>);
  ...when !=fd=C
* if (fd<0||...){...}
...+>  
 
@script:python@
p << search.p;
@@
 
print "%s equal expression" % (p[0].line)
</code></pre>

之后我们可以运行这个脚本，可以快速从代码中匹配。

<pre><code>	
$spatch -sp_file demos/simple.cocci demos/simple.c -o /tmp/new_simple.c
</code></pre>

目前这个项目的问题是文档不是很完善，期待之后这个项目的发展。这个工具吸引人的地方在于可以智能的匹配譬如i++ <=> i=i+1这种形式。

目前我们可以更多的参考/usr/local/share/coccinelle/standard.iso

