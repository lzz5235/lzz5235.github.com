---
layout: post
title: "基于Redis的分布式锁的思考"
categories: Redis,Distribution System
tags: Java
---
最近在实现一个分布式锁，网上有很多种版本，有的是使用zk实现，有的是使用db，我这里主要使用的是redis。

网上对于基于redis的分布式锁有很多版本，有的实现是错误的，需要进行辨别，我这里主要依赖的是redis的特性，setnx在原本实现里，是没有过期时间的，因此如果遇到系统没有解锁，就会导致死锁，因此网上针对这种问题衍生了很多版本，比如在value里面塞入expireTime来保证是否过期。

但是这种实现方式导致代码很冗余，读起来很晦涩，因此在redis在2.6.12后，set支持setnx的操作，并且支持过期时间，另外该原语是原子的，因此实现分布式锁就两行代码，比网上的实现简化了很多。

<pre><code>
    Set key to hold the string value. If key already holds a value, it is overwritten, regardless of its type. Any previous time to live associated with the key is discarded on successful SET operation.

    Options
    Starting with Redis 2.6.12 SET supports a set of options that modify its behavior:

    EX seconds — Set the specified expire time, in seconds.
    PX milliseconds — Set the specified expire time, in milliseconds.
    NX — Only set the key if it does not already exist.
    XX — Only set the key if it already exist.
    Note: Since the SET command options can replace SETNX, SETEX, PSETEX, it is possible that in future versions of Redis these three commands will be deprecated and finally removed.

    Return value
    Simple string reply: OK if SET was executed correctly. Null reply: a Null Bulk Reply is returned if the SET operation was not performed because the user specified the NX or XX option but the condition was not met.
</code></pre>

在很多不是太注重高并发的场景下还是可以当用的，但使用这种分布式锁还是有些问题的，下面这种场景就是一种情况.

![](/assets/pic/unsafe-lock.png)

假设锁服务本身是没有问题的，它总是能保证任一时刻最多只有一个客户端获得锁。上图中出现的lease这个词可以暂且认为就等同于一个带有自动过期功能的锁。客户端1在获得锁之后发生了很长时间的GC pause，在此期间，它获得的锁过期了，而客户端2获得了锁。当客户端1从GC pause中恢复过来的时候，它不知道自己持有的锁已经过期了，它依然向共享资源（上图中是一个存储服务）发起了写数据请求，而这时锁实际上被客户端2持有，因此两个客户端的写请求就有可能冲突（锁的互斥作用失效了）。

因此一种方法，称为fencing token。fencing token是一个单调递增的数字，当客户端成功获取锁的时候它随同锁一起返回给客户端。而客户端访问共享资源的时候带着这个fencing token，这样提供共享资源的服务就能根据它进行检查，拒绝掉延迟到来的访问请求（避免了冲突）

![](/assets/pic/fencing-tokens.png)

虽然解决了该问题，但是本质上是因为Redlock的安全性(safety property)对系统的时钟有比较强的依赖，一旦系统的时钟变得不准确，算法的安全性也就保证不了了。Martin在这里其实是要指出分布式算法研究中的一些基础性问题，或者说一些常识问题，即好的分布式算法应该基于异步模型(asynchronous model)，算法的安全性不应该依赖于任何记时假设(timing assumption)。

*    如果是为了效率(efficiency)而使用分布式锁，允许锁的偶尔失效，那么使用单Redis节点的锁方案就足够了，简单而且效率高。Redlock则是个过重的实现(heavyweight)。
*    如果是为了正确性(correctness)在很严肃的场合使用分布式锁，那么不要使用Redlock。它不是建立在异步模型上的一个足够强的算法，它对于系统模型的假设中包含很多危险的成分(对于timing)。而且，它没有一个机制能够提供fencing token。那应该使用什么技术呢？Martin认为，应该考虑类似Zookeeper的方案，或者支持事务的数据库。

##参考

https://blog.csdn.net/jek123456/article/details/72954106

https://redis.io/commands/set