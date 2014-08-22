---
layout: post
title: "Git Standard Development Model"
categories: others
tags: git

---
Git标准开发模型
=========

之前用了很长时间的git，最近才了解到git的正确用法。

正如Nicholas said:Using git is very easy but using git correctly is not easy!

git文件最好是以二进制的存在，这样比较容易对文件进行追踪。最好不要将binary文件add进来！doc？exe？No！


http://longair.net/blog/2009/04/16/git-fetch-and-merge/

http://www.oschina.net/translate/git-fetch-and-merge?cmp


我们拿这篇文章说，很多时候，我们用git，把他当成svn了。因为我们知道svn只能同时有一位开发者在操作文件，其他文件是锁定的。

我们常常git pull 这个就和svn checkout差不多。但是我们要谨慎使用git pull （git pull = git fetch + get merge）

但是git是并行开发的。

在linux kernel中git广泛使用，比如git的master分支只有少数人有权限可以写，其他人只能进行读。控制master的分支读写的人也就是linus这种人了吧。。。。 :-D

对于我们，我们只能clone他的master分支和有dev branch。

对于linux kernel这么大的项目，git才能大显身手。

看下面的图：

![](/assets/pic/git-1024x374.png)


我们看到CM machine是主分支，上面的/shared/test.git就是类比于master，而linus控制着Local work test。他们之间使用git clone操作，如图所示。

如果我们要对linux kernel的master进行修改，我们必须要clone master分支，然后我们可以在本地通过–bare建一个git server。

我们日常的修改就是在本地的local work test 和本地服务器之间进行。

如果我们开发好了一个东西，我们要首先和管理master的管理员联系。说我们要合并分支。

> 然后CM machine管理员通过git fetch去fetch 开发者在本地的git 服务器。
这时管理员可以在本地看到FETCH_HEAD和log ，这时我们通过git merge合并分支。然后由他们把你的修改push到master中。

> 如果他对fetch的内容不同意，他就会弃置不管，不会进行merge！

我们可以先使用git log -p HEAD..FETCH查看更改。然后使用git merge MD5  合并每次提交。

完成对源代码的修改。

 
要记住，在push到远程仓库前，我们有后悔药吃，我们可以用git revert 和git reset。第一个比第二个安全！

第一个会在git log 中留下记录，第二个就会直接回滚到指定的commit！

git reset HEAD^ 回到前一个提交，不会在log中留下记录。

所以要谨慎使用！

附自己画的图：

![](/assets/pic/IMG_09552.jpg)
