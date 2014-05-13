---
layout: post
title: "Factory_DesignPattern"
categories: [DesignPattern]
tags: [C/C++]
---
三种工厂模式的C++实现
=======================
最近想换换口味，于是抱起久违的design pattern漫画书品读起来。
工厂模式属于创建型模式，大致可以分为三类，简单工厂模式、工厂方法模式、抽象工厂模式。

首先介绍简单工厂模式，它的主要特点是需要在工厂类中做判断，从而创造相应的产品。当增加新的产品时，就需要修改工厂类。
缺点：对修改不封闭，新增加产品您要修改工厂。违法了鼎鼎大名的开闭法则（OCP）。

![](/assets/pic/SingleFactory.png)
下面的代码，由于插件有些问题，导致无法在网页显示，我将尽快修复！
<pre><code>
#include <iostream>
using namespace std;
 
enum TYPE{COREA,COREB};

class SingleCore
{
    public:
        virtual ~SingleCore(){};
        virtual void Show() = 0;
};
 
class CoreA:public SingleCore
{
    public:
        void Show()
        {
            cout<< "CoreA"<< endl;
        }
};
class CoreB:public SingleCore
{
    public:
        void Show()
        {
            cout <<"CoreB"<< endl;
        }
};
 
class SingleFactory
{
    public:
        SingleCore* CreateSingleCore(enum TYPE type)
        {
            if(type==COREA)
                return new CoreA();
            else if(type==COREB)
                return new CoreB();
            else 
                return NULL;
        }
};
 
 
int main()
{
    SingleFactory factory;
    SingleCore  *pcorea = factory.CreateSingleCore(COREA);
    pcorea->Show();
    SingleCore  *pcoreb = factory.CreateSingleCore(COREB);
    pcoreb->Show();
 
    delete pcorea;
    delete pcoreb;
}
</code></pre>

工厂方法模式的应用并不是只是为了封装对象的创建，而是要把对象的创建放到子类中实现：Factory中只是提供了对象创建的接口，其实现将放在Factory的子类Factory中进行。
工厂方法模式也有缺点，每增加一种产品，就需要增加一个对象的工厂。

![](/assets/pic/Factory.png)
<pre><code>

#include <iostream>
using namespace std;
 
 
class SingleCore
{
    public:
        virtual ~SingleCore(){};
        virtual void Show() = 0;
};
 
class CoreA:public SingleCore
{
    public:
        void Show()
        {
            cout<< "CoreA"<< endl;
        }
};
class CoreB:public SingleCore
{
    public:
        void Show()
        {
            cout <<"CoreB"<< endl;
        }
};
 
class SingleFactory
{
    public:
        virtual SingleCore* CreateSingleCore() = 0;
};
class FactoryA:public SingleFactory
{
    public:
        SingleCore* CreateSingleCore()
        {
            return new CoreA;
        }
};
 
class FactoryB:public SingleFactory
{
    public:
        SingleCore* CreateSingleCore()
        {
            return new CoreB;
        }
};
 
 
int main()
{
    FactoryA factorya;
    SingleCore  *pcorea = factorya.CreateSingleCore();
    pcorea->Show();
 
    FactoryB factoryb;
    SingleCore  *pcoreb = factoryb.CreateSingleCore();
    pcoreb->Show();
 
    delete pcorea;
    delete pcoreb;
}
</code></pre>
抽象工厂模式登场了。它的定义为提供一个创建一系列相关或相互依赖对象的接口，而无需指定它们具体的类。
抽象工厂模式的组成（和工厂方法模式一样）：

1)抽象工厂角色：这是工厂方法模式的核心，它与应用程序无关。是具体工厂角色必须实现的接口或者必须继承的父类。

2)具体工厂角色：它含有和具体业务逻辑有关的代码。由应用程序调用以创建对应的具体产品的对象。

3)抽象产品角色：它是具体产品继承的父类或者是实现的接口。

4)具体产品角色：具体工厂角色所创建的对象就是此角色的实例。

![](/assets/pic/AbstractFactory.png)

<pre><code>

#include <iostream>
using namespace std;
 
 
class SingleCore
{
    public:
        virtual ~SingleCore();
        virtual void Show() = 0;
};
 
class CoreA:public SingleCore
{
    public:
        void Show()
        {
            cout<< "CoreA"<< endl;
        }
};

class CoreB:public SingleCore
{
    public:
        void Show()
        {
            cout <<"CoreB"<< endl;
        }
};
//-------------------------------------------------------
class MultiCore
{
    public:
        virtual void Show() = 0;
        virtual ~MultiCore(){};
};
 
class MultiCoreA:public MultiCore
{
    public:
        void Show()
        {
            cout<<"MultiCoreA"<< endl;
        }
};
 
class MultiCoreB:public MultiCore
{
    public:
        void Show()
        {   
            cout<<"MultiCoreB"<< endl;
        }   
};
//----------------------------------------------------------
class CoreFactory
{
    public:
        virtual SingleCore* CreateSingleCore() = 0;
        virtual MultiCore* CreateMultiCore() = 0;
};
class FactoryA:public CoreFactory
{
    public:
        SingleCore* CreateSingleCore()
        {
            return new CoreA();
        }
        MultiCore* CreateMultiCore()
        {
            return new MultiCoreA();
        }
};
 
class FactoryB:public CoreFactory
{
    public:
        SingleCore* CreateSingleCore()
        {
            return new CoreB();
        }
        MultiCore* CreateMultiCore()
        {
            return new MultiCoreB();
        }
 
};
 
 
int main()
{
    FactoryA factorya;
    SingleCore  *pcorea = factorya.CreateSingleCore();
    pcorea->Show();
 
    MultiCore   *pmulticorea = factorya.CreateMultiCore();
    pmulticorea->Show();
 
    FactoryB factoryb;
    SingleCore  *pcoreb = factoryb.CreateSingleCore();
    pcoreb->Show();
 
    MultiCore   *pmulticoreb = factoryb.CreateMultiCore();
    pmulticoreb->Show();
 
    delete pcorea;
    delete pcoreb;
    delete pmulticorea;
    delete pmulticoreb;
}

</code></pre>


后续我会学习其他设计模式。并贴出C++代码实现。

参考：http://blog.csdn.net/silangquan/article/details/20492293
