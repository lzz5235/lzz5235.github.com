---
layout: post
title: "Flyweight Pattern with C++"
categories: [designpattern]

tags: [C/C++]
---
享元模式 C++实现
==================
享元模式可以避免大量非常相似类的开销。在程序设计中，有时需要生成大量细粒度的类实例来表示数据。如果能发现这些实例数据除了几个参数外基本都是相同的。有时就能够大幅度地减少实例化的类的数量。如果能把那些参数移到类实例的外面，在方法调用时将它们传递进来，就可以通过共享大幅度地减少单个实例的数目。


这种模式可以运用共享技术有效地支持大量细粒度的对象。适合具有缓冲池的场景。

首先我们要分清楚享元模式的内部状态和外部状态。

内部状态：在享元对象内部并且不会随环境改变而改变的共享部分，可以称为是享元对象的内部状态，比如base class中的member

外部状态：而随环境改变而改变的，取决于应用环境，或是实时数据，这些不可以共享的东西就是外部状态了。比如存在的map的key值

内部状态存储于ConcreteFlyweight对象之中；而外部状态则由Client对象存储或计算。

####当用户调用Flyweight对象的操作时，将该状态传递给它。

####外部状态有一个factory，负责维护外部状态，并把外部状态和class放入container中。

![](/assets/pic/flyweight.png)

<pre><code>
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <hash_map>
using namespace std;
 
class Flyweight
{
public:
    //操作外部状态extrinsicState
    virtual void Operation(const string& extrinsicState)=0;
    string GetIntrinsicState()
    {
        return this->_intrinsicState;
    }
    virtual ~Flyweight(){}
protected:
    Flyweight(string intrinsicState)
    {
        this->_intrinsicState = intrinsicState;
    }
private:
    //内部状态，也可以放在ConcreteFlyweight中
    string _intrinsicState;
};
 
class ConcreteFlyweight:public Flyweight
{
public:
    virtual void Operation(const string& extrinsicState)
    {
        cout << this->GetIntrinsicState() << endl;
        cout << extrinsicState << endl;
    }
    ConcreteFlyweight(string intrinsicState):Flyweight(intrinsicState){}
    ~ConcreteFlyweight(){}
};
 
class UnsharedConcreteFlyweight:public Flyweight
{
public:
    virtual void Operation(const string& extrinsicState)
    {
        cout << extrinsicState << endl;
    }
    UnsharedConcreteFlyweight(string intrinsicState):Flyweight(intrinsicState){}
    ~UnsharedConcreteFlyweight(){}
};
 
class FlyweightFactory
{
public:
    FlyweightFactory(){}
    ~FlyweightFactory(){}
    //获得一个请求的Flyweight对象
    Flyweight* GetFlyweight(string key)
    {
        if(m_mapFly.find(key)!=m_mapFly.end())
        {
            return m_mapFly[key];
        }
        //Flyweight* fly = new ConcreteFlyweight(key);
        Flyweight* fly = new UnsharedConcreteFlyweight(key);
        this->m_mapFly.insert(pair<string,Flyweight*>(key,fly));
        return fly;
    }
    //获取容器中存储的对象数量
    void GetFlyweightCount()
    {
        cout << this->m_mapFly.size() << endl;
    }
protected:
private:
    //保存内部状态对象的容器
    hash_map<string,Flyweight*> m_mapFly;
};
 
int main()
{
    //外部状态extrinsicState
    string extrinsicState = "ext";
 
    //工厂对象，工厂对象
    FlyweightFactory* fc = new FlyweightFactory();
 
    Flyweight* fly = fc->GetFlyweight("hello");
    fly = fc->GetFlyweight("hello1");
 
    fly = fc->GetFlyweight("hello");
 
    //应用外部状态
    fly->Operation("hello3");
 
    fc->GetFlyweightCount();
 
    return 0;
}
</code></pre>

线程环境下，必须使用锁变量。结局这种问题的方式，就是使用外部类来当做key值，但是这又会出现另一个问题：维护一个container。class的效率明显比string类型低。

参考：

[http://blog.csdn.net/lcl_data/article/details/8974679](http://blog.csdn.net/lcl_data/article/details/8974679)

[http://msdn.microsoft.com/zh-cn/library/th79x793.aspx](http://msdn.microsoft.com/zh-cn/library/th79x793.aspx)
