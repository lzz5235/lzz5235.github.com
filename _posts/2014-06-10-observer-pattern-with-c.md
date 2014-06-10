---
layout: post
title: "Observer Pattern with C++"
categories: [designpattern]

tags: [C/C++]
---
观察者模式C++实现
===================
观察者模式是非常重要的一个模式，初次遇到该模式，想了很久没有想通应该怎么做。

其实原理很简单：就是当被观察者（Observable）发生变化，然后就会某种数据方式通知（Notify）观察者（Observer），Notify的方式很多，比如我们将Observer 放到一个list或者set中，分别调用Observer的update（）这样实现通知。

这个模式和之前的责任链模式很相似，不同的是观察者模式在消息传播的时候，消息是可变的。责任链模式在消息 传播的过程保持消息的不可变。

####另外我们要说明的是ObserverPattern模式中最好只出现一个既是Observer又是Observable的对象，消息最多转发一次

####从效率上来说，如果一个Observer被阻塞，会影响其他Observer的Notify（），所以Notify（）最好采用异步的方式。不过异步的话就要考虑线程安全和队列的问题，需要查看message queue。

下面是UML类图
![](/assets/pic/Observer.png)

<pre><code>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
 
class IObserver
{
public:
    IObserver(string _name)
    {
        this->m_name = _name;
    }
    virtual ~IObserver(void)
    {
    }
    virtual void Update(string context) = 0;
    virtual string GetName() = 0;//为c++单独增加的函数，用于删除时查找观察者。
protected:
    string m_name;
};
 
class IObservable
{
public:
    IObservable(void)
    {
    }
    virtual ~IObservable(void)
    {
    }
    virtual void AddObserver(IObserver *pObserver) = 0;
    virtual void DeleteObserver(IObserver *pObserver) = 0;
    virtual void NotifyObservers(string context) = 0;
};
 
class SuperStar : public IObservable
{
public:
    SuperStar(void){}
    ~SuperStar(void){}
    void AddObserver(IObserver *pObserver){m_observerList.push_back(pObserver);}
    void DeleteObserver(IObserver *pObserver)
    {
        ObserverList_C_iterator it = m_observerList.begin();
        for (; it != m_observerList.end(); it++)
        {
            string name = (*it)->GetName();
            if (name.compare(pObserver->GetName()) == 0)
            {
                it = m_observerList.erase(it);
            }
        }
    }
    void NotifyObservers(string context)
    {
        ObserverList_C_iterator it = m_observerList.begin();
        for (; it != m_observerList.end(); it ++)
        {
            (*it)->Update(context);
        }
    }
    void HaveBreakfast()
    {
         cout << "SuperStart begin to have breakfast..." << endl;
         this->NotifyObservers("SuperStart begin to have breakfast");
    }
    void HaveFun()
    {
         cout << "SuperStart begin to have fun..." << endl;
         this->NotifyObservers("SuperStart begin to have fun");
    }
private:
    vector<IObserver*> m_observerList;
    typedef vector<IObserver*>::const_iterator ObserverList_C_iterator;
};
class Spy1 :  public IObserver
{
public:
    Spy1(void):IObserver("SPY1"){}
    ~Spy1(void){}
    void Update(string context)
    {
        cout << "Spy1:something happened..." << endl;
        this->ReportToBoss(context);
        cout << "Spy1:get some price" << endl;
    }
    string GetName(){return m_name;}
private:
    void ReportToBoss(string report)
    {
         cout << "Spy1:Boss->" << report.c_str() << endl;
    }
};
class Spy2 :  public IObserver
{
public:
    Spy2(void):IObserver("SPY1"){}
    ~Spy2(void){}
    void Update(string context)
    {
        cout << "Spy2:something happened..." << endl;
        this->ReportToBoss(context);
        cout << "Spy2:get some price" << endl;
    }
    string GetName(){return m_name;}
private:
    void ReportToBoss(string report)
    {
         cout << "Spy2:Boss->" << report.c_str() << endl;
    }
};
 
int main()
{
    IObserver *spy1 = new Spy1();
    IObserver *spy2 = new Spy2();
 
    SuperStar *superstar = new SuperStar();
 
    superstar->AddObserver(spy1);
    superstar->AddObserver(spy2);
 
    superstar->HaveFun();
 
    cout << endl;
    superstar->DeleteObserver(spy1);
    superstar->HaveBreakfast();
 
    delete spy1;
    spy1 = NULL;
    delete spy2;
    spy2 = NULL;
}
</code></pre>

因为是C++实现，没有通用的Observer Pattern framework，所以我们在class superstar中实现了

void AddObserver(IObserver *pObserver)

void DeleteObserver(IObserver *pObserver)

void NotifyObservers(string context)

如果我们把这个代码变为一种framework，类图会变化
![](/assets/pic/Observer2.png)
