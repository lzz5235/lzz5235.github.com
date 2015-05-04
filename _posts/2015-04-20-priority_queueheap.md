---
layout: post
title: "priority_queue与heap的使用"
categories: others
tags: STL
---
priority_queue与heap的使用
=========================
1.priority_queue

priority_queue是一个优先队列,下面是他的声明，我们平时可以直接使用下面的方式声明一个优先队列。

<pre><code>
priority_queue<int> pq
</code></pre>

优先队列内部是一个heap的实现，也就是说默认push到priority_queue中的数据，当我们pop出来的时候，默认是优先级最高的,(数字大的优先级高，数字小的优先级低)，这个数据结构默认使用vector作为容器，cmp函数默认使用less作为比较函数。

下面的是一个完整的priority_queue的声明

<pre><code>
std::priority_queue
template < class T, class Container = vector< T >,
class Compare = less < typename Container::value_type> > class priority_queue;
</code></pre>

我们使用的时候和平常queue的方式没有什么太大的却别，最大的区别在于这个cmp应该如何自定义。我们知道cmp是一个函数指针，所以我们可以有两种方式重载cmp函数。

方式1：

<pre><code>
struct Time {
    int h;
    int m;
    int s;
};
 
class CompareTime {
    public:
    bool operator()(Time& t1, Time& t2) // Returns true if t1 is earlier than t2
    {
       if (t1.h < t2.h) return true;
       if (t1.h == t2.h && t1.m < t2.m) return true;
       if (t1.h == t2.h && t1.m == t2.m && t1.s < t2.s) return true;
       return false;
    }
}
</code></pre>

这里我们必须保证重载的()函数返回值是bool，上面的重载函数核心就是当t1<t2时候，返回tree，所以得到的也就是从大到小的排列，也是这个数据结构默认的，如果我们想重新实现这个数据结构，改为从小到大排列，那么可以使用下面的方式

方式2：

<pre><code>
class CompareTime {
public:
    bool operator()(Time& t1, Time& t2) // t2 has highest prio than t1 if t2 is earlier than t1
    {
       if (t1.h > t2.h) return true;
       if (t2.h == t1.h && t2.m < t1.m) return true;
       if (t2.h == t1.h && t2.m == t1.m && t2.s < t1.s) return true;
       return false;
    }
};
</code></pre>

保证第一个大于第二个返回true即可。
上面我们看到在一个class类里面重载()函数，我们也可以在要使用的类里面，使用struct{}方式。

<pre><code>
class Solution {
public:
.....
private:
struct cmp {
        bool operator()(ListNode* node1, ListNode* node2) {
            return node1->val > node2->val;
        }
    };
};
</code></pre>

在C/C++中，我们可以等同class与struct相似。

2.heap
heap 主要分为push_heap、pop_heap、sort_heap、reverse四个函数，我们使用这四个函数使得vector中数据按照heap来排列。

make_heap的两种形式：

<pre><code>
template < class RandomAccessIterator >
  void make_heap (RandomAccessIterator first, RandomAccessIterator last);
template < class RandomAccessIterator, class Compare >
  void make_heap (RandomAccessIterator first, RandomAccessIterator last,
                  Compare comp );
</code></pre>

同样有一个comp函数可以指定以排列顺序，所以priority_queue是基于heap的方式来实现的。

 

参考：

[1] [http://comsci.liu.edu/~jrodriguez/cs631sp08/c++priorityqueue.html](http://comsci.liu.edu/~jrodriguez/cs631sp08/c++priorityqueue.html)

[2] [http://www.cplusplus.com/reference/queue/priority_queue/](http://www.cplusplus.com/reference/queue/priority_queue/)

[3] [http://stackoverflow.com/questions/23529815/how-to-use-stdmake-heap](http://stackoverflow.com/questions/23529815/how-to-use-stdmake-heap)