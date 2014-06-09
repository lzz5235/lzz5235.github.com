---
layout: post
title: "Strategy Pattern with C++"
categories: [designpattern]

tags: [C/C++]
---
策略模式及其拓展 C++实现
==========================
策略模式形式上其实和装饰者模式，工厂模式都很像，装饰者模式是在原有核心的业务上添加新的业务，工厂模式是在构造函数中传入新类类名，然后返回新的类。而策略模式则是将策略类传入到执行类里，达到执行策略的目的！

工厂模式和策略模式有很多相通的地方。都是创造类。

######不同点在于：工厂模式有可能各个类的方法名和个数差别很大，注重不同对象的创建。策略模式 一般用于多个类的方法名都相同,但是实现方式不同 注重多个对象的相同行为：屏蔽方法名相同，算法实现细节不同之间的差异

策略模式的缺点是：策略类数量过多，所有策略类都必须对外暴露。

#####原则就是：当具体策略超过4个的时候，就要考虑使用混合模式，比如和工厂模式混用！
![](/assets/pic/1855.png)

<pre><code>
#include <iostream>
#include <string>
using namespace std;
 
class IStrategy
{
public:
    IStrategy(void){};
    virtual ~IStrategy(void){};
    virtual void Operate(void) = 0;
};
 
class context
{
public:
    context(IStrategy *isstrategy):m_strategy(isstrategy){}
    ~context(){}
    void Operate(void){m_strategy->Operate();}
private:
    IStrategy *m_strategy;
      
};
 
class BackDoor:public IStrategy
{
public:
    BackDoor(void){}
    ~BackDoor(void){}
    void Operate(void){std::cout <<"BackDoor" << std::endl;}
};
 
class GreenLight:public IStrategy
{
public:
    GreenLight(void){}
    ~GreenLight(void){}
    void Operate(void){std::cout << "GreenLight" << std::endl;}
};
 
int main()
{
    context*context1 =  new context(new BackDoor());
    context1->Operate();
 
    context*context2 =  new context(new GreenLight());
    context2->Operate();
 
    return 0;
}
</code></pre>

因为这个模式过于简单,_____所以我们要配合其他模式使用，如果我们配合工厂模式，正好可以弥补策略模式的缺点！_____

我增加了class Policeoffice 与 class Police

其中class Policeoffice主要是下达具体命令的地方，这也是工厂模式的精髓！class Police封装这个工厂，并使用创造的新类执行。

#####这个完美的弥补了策略模式，策略类太多的弊病。策略类多正好由工厂区创建！

![](/assets/pic/0551.png)

<pre><code>
#include <iostream>
#include <string>
using namespace std;
 
class IStrategy
{
public:
    IStrategy(void){};
    virtual ~IStrategy(void){};
    virtual void Operate(void) = 0;
};
 
class Context
{
public:
    Context(IStrategy *isstrategy):m_strategy(isstrategy){}
    ~Context(){}
    void Operate(void){m_strategy->Operate();}
private:
    IStrategy *m_strategy;
      
};
 
class BackDoor:public IStrategy
{
public:
    BackDoor(void){}
    ~BackDoor(void){}
    void Operate(void){std::cout <<"BackDoor" << std::endl;}
};
 
class GreenLight:public IStrategy
{
public:
    GreenLight(void){}
    ~GreenLight(void){}
    void Operate(void){std::cout << "GreenLight" << std::endl;}
};
 
class Policeoffice
{
public:
    Policeoffice(){}
    ~Policeoffice(){}
    IStrategy* getStrategy(string classname)
    {
        if(classname =="BackDoor")
            return new BackDoor();
        else if(classname =="GreenLight")
            return new GreenLight();
 
        return NULL;
    }
};
 
class Police
{
public:
    Police(){office = new Policeoffice();}
    ~Police(){delete office;}
    void PoliceDeduct(string classname)
    {       
        IStrategy *strategy = office->getStrategy(classname);
        context = new Context(strategy);
        context->Operate();
    }
private:
    Policeoffice *office;
    IStrategy *strategy;
    Context *context;
};
 
int main()
{
    Police police;
    police.PoliceDeduct("BackDoor");
 
    return 0;
}
</code></pre>

设计模式不止一种，所以我们要考虑混用各种设计模式，达到解耦的效果！

####封装的精髓就是避免高层模块深入系统内部，这样使得系统达到高内聚，低耦合的特性。