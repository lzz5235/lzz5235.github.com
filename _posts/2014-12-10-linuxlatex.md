---
layout: post
title: "Linux下安装LaTeX环境"
categories: others
tags: LaTex
---
Linux下安装LaTeX环境
====================
由于我使用的是Fedora 20 ，可以直接使用yum机制安装。Ubuntu/Debian 用户可以使用apt-get安装。

Linux主要存在的问题是众多fonts文件的缺失，所以当我们编译中文pdf时，经常会出现fonts cannot found的现象。本文主要解决这个问题：

从别处拷贝需要的字体，从Windows的C:/Windows/Fonts下拷贝最常使用的几种字体：msyh.ttf（微软雅黑）  simfang.ttf（仿宋）  simhei.ttf（黑体）  simkai.ttf（楷体）  simsun.ttc（宋体）。

拷贝到Linux的/usr/share/fonts/winfonts/目录下

<pre><code>
#chmod a+x  /usr/share/fonts/winfonts/
 
# cd /usr/share/fonts/winfonts/
# mkfontscale
# mkfontdir
# fc-cache
</code></pre>

拷贝完成之后一定要注意使用chmod给字体权限,为X字体创建可裁剪的索引，在一个目录中创建X字体文件的索引，建立字体信息缓存文件。
使用fc-list 命令查看：

<pre><code>
➜  TIR_CN git:(master) ✗ fc-list :lang=zh-cn
/usr/share/fonts/winfonts/simsun.ttc: 新宋体,NSimSun:style=Regular
/usr/share/fonts/wqy-zenhei/wqy-zenhei.ttc: 文泉驿点阵正黑,文泉驛點陣正黑,WenQuanYi Zen Hei Sharp:style=Regular
/usr/share/fonts/adobe-source-han-sans-cn/SourceHanSansCN-Regular.otf: 思源黑体 CN,Source Han Sans CN,Source Han Sans CN Regular,思源黑体 CN Regular:style=Regular
/usr/share/fonts/google-droid/DroidSansFallback.ttf: Droid Sans:style=Regular
/usr/share/fonts/winfonts/simkai.ttf: 楷体,KaiTi:style=Regular,Normal,obyčejné,Standard,Κανονικά,Normaali,Normál,Normale,Standaard,Normalny,Обычный,Normálne,Navadno,Arrunta
/usr/share/fonts/winfonts/simfang.ttf: 仿宋,FangSong:style=Regular,Normal,obyčejné,Standard,Κανονικά,Normaali,Normál,Normale,Standaard,Normalny,Обычный,Normálne,Navadno,Arrunta
/usr/share/fonts/adobe-source-han-sans-cn/SourceHanSansCN-ExtraLight.otf: 思源黑体 CN,Source Han Sans CN,Source Han Sans CN ExtraLight,思源黑体 CN ExtraLight:style=ExtraLight,Regular
/usr/share/fonts/adobe-source-han-sans-cn/SourceHanSansCN-Normal.otf: 思源黑体 CN,Source Han Sans CN,Source Han Sans CN Normal,思源黑体 CN Normal:style=Normal,Regular
/usr/share/fonts/cjkuni-uming/uming.ttc: AR PL UMing TW MBE:style=Light
/usr/share/fonts/winfonts/simsun.ttc: 宋体,SimSun:style=Regular
/usr/share/fonts/winfonts/simhei.ttf: 黑体,SimHei:style=Regular,Normal,obyčejné,Standard,Κανονικά,Normaali,Normál,Normale,Standaard,Normalny,Обычный,Normálne,Navadno,Arrunta
/usr/share/fonts/adobe-source-han-sans-cn/SourceHanSansCN-Medium.otf: 思源黑体 CN,Source Han Sans CN,Source Han Sans CN Medium,思源黑体 CN Medium:style=Medium,Regular
/usr/share/fonts/gnu-free/simhei.ttf: 黑体,SimHei:style=Regular
........
/usr/share/fonts/winfonts/simsun.ttc: 宋体,SimSun:style=Regular
</code></pre>

我们可以看到中文后面的那个SimSum就是在linux中的字体符号.

我们打开/usr/share/texlive/texmf-dist/tex/latex/ctex/fontset/ctex-xecjk-winfonts.def
修改我们缺少的字体。

<pre><code>
% ctex-xecjk-winfonts.def: Windows 的 xeCJK 字体设置，默认为六种中易字体
% vim:ft=tex
 
\setCJKmainfont[BoldFont={SimHei},ItalicFont={KaiTi}]
  {SimSun}
\setCJKsansfont{SimHei}
\setCJKmonofont{FangSong}
 
\setCJKfamilyfont{zhsong}{SimSun}
\setCJKfamilyfont{zhhei}{SimHei}
\setCJKfamilyfont{zhkai}{KaiTi}
\setCJKfamilyfont{zhfs}{FangSong}
% \setCJKfamilyfont{zhli}{LiSu}
% \setCJKfamilyfont{zhyou}{YouYuan}
 
\newcommand*{\songti}{\CJKfamily{zhsong}} % 宋体
\newcommand*{\heiti}{\CJKfamily{zhhei}}   % 黑体
\newcommand*{\kaishu}{\CJKfamily{zhkai}}  % 楷书
\newcommand*{\fangsong}{\CJKfamily{zhfs}} % 仿宋
% \newcommand*{\lishu}{\CJKfamily{zhli}}    % 隶书
% \newcommand*{\youyuan}{\CJKfamily{zhyou}} % 幼圆
 
\endinput
</code></pre>

然后大功告成，可以使用xelatex生成pdf文件了。

参考文档

【1】 [http://www.tug.org/texlive/doc/texlive-zh-cn/texlive-zh-cn.pdf](http://www.tug.org/texlive/doc/texlive-zh-cn/texlive-zh-cn.pdf)

【2】 [http://www.aboutlinux.info/2005/12/adding-windows-fonts-in-linux.html](http://www.aboutlinux.info/2005/12/adding-windows-fonts-in-linux.html)

【3】 [http://hi.baidu.com/defeattroy/blog/item/5df1ffcac3348415be09e6de.html](http://hi.baidu.com/defeattroy/blog/item/5df1ffcac3348415be09e6de.html)

【4】 [http://learningloong.blog.163.com/blog/static/1623081242011731102219361/](http://learningloong.blog.163.com/blog/static/1623081242011731102219361/)

【5】 [http://forum.ubuntu.com.cn/viewtopic.php?f=35&t=350882](http://forum.ubuntu.com.cn/viewtopic.php?f=35&t=350882)

【6】[http://blog.csdn.net/defeattroy/article/details/7581846](http://blog.csdn.net/defeattroy/article/details/7581846)
