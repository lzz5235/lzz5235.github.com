---
layout: post
title: "Bridge Pattern with C++"
categories: [designpattern]

tags: [C/C++]
---
桥梁模式 C++实现
=================
桥梁模式主要解决多层继承，比如我们可以想想，人可以分为男人女人，人又有不同的行为。如果把这个两个class继承，也就意味着是男人行为A，男人行为B。女人行为A，女人行为B。
这种方式非常难以扩展，所以桥梁模式派上用场，它将表示和实现解耦,两者可以独立的变化。
#####也就是抽象出人和行为，然后人调用行为，减少了继承的class。

其中这种模式又有一些像策略模式，又有一些像建造者模式。

#####当我们考虑一个对象的多个变化因素可以动态变化的时候，考虑使用桥接模式

优点

1.将实现抽离出来，再实现抽象，使得对象的具体实现依赖于抽象，满足了依赖倒转原则。

2.将可以共享的变化部分，抽离出来，减少了代码的重复信息。

3.对象的具体实现可以更加灵活，可以满足多个因素变化的要求。

缺点

1.客户必须知道选择哪一种类型的实现。


如果我们用山寨工厂和房地产工厂生产产品做例子，非常清晰易懂。

####这里我们要知道class IProduct中有两个虚基类，用于动态绑定！

![](/assets/pic/bridge.png)

<pre><code>
#include <iostream>
#include <string>
#include <list>
using namespace std;
 
class IProduct
{
public:
    IProduct(void)
    {
    }
    virtual ~IProduct(void)
    {
    }
    virtual void BeProducted() = 0;
    virtual void BeSelled() = 0;
};
 
class CNewCorp
{
public:
    CNewCorp(IProduct *pproduct)
    {
        this->m_pProduct = pproduct;
    }
    virtual ~CNewCorp(void){}
    void MakeMoney()
    {
        //每个公司都是一样，先生产
        this->m_pProduct->BeProducted();
        //然后销售
        this->m_pProduct->BeSelled();
    }
private:
    IProduct *m_pProduct;
};
class CHouse : public IProduct
{
public:
    CHouse(void){}
    ~CHouse(void){}
    void BeProducted(){cout << "生产出的房子是这个样子的..." << endl;}
    void BeSelled(){cout << "生产出的房子卖出去了..." << endl;}
};
 
class CClothes : public IProduct
{
public:
    CClothes(void){}
    ~CClothes(void){}
    void BeProducted(){cout << "生产出的衣服是这个样子的..." << endl;}
    void BeSelled(){cout << "生产出的衣服卖出去了..." << endl;}
};
 
class CNewHouseCorp : public CNewCorp
{
public:
    CNewHouseCorp(CHouse *pHouse):CNewCorp(pHouse){}
    ~CNewHouseCorp(void){}
    void MakeMoney()
    {
        this->CNewCorp::MakeMoney();
        cout << "房地产公司赚大钱了..." << endl;
    }
};
 
class CIPod :public IProduct
{
public:
    CIPod(void){}
    ~CIPod(void){}
    void BeProducted(){cout << "生产出的ipod是这个样子的..." << endl;}
    void BeSelled(){ cout << "生产出的ipod卖出去了..." << endl;}
};
 
class CShanZhaiCorp : public CNewCorp
{
public:
    CShanZhaiCorp(IProduct *pproduct): CNewCorp(pproduct){}
    ~CShanZhaiCorp(void){}
    void MakeMoney()
    {
        this->CNewCorp::MakeMoney();
        cout << "我赚钱呀..." << endl;
    }
};
void DoNewRun1()
{
    cout << "----------房地产公司是这样运行的----------" << endl;
    CHouse house;
    CNewHouseCorp newHouseCorp(&house);
    newHouseCorp.MakeMoney();
    cout << endl;
 
    cout << "----------山寨公司是这样运行的----------" << endl;
    CClothes clothes;
    CShanZhaiCorp shanZhaiCorp(&clothes);
    shanZhaiCorp.MakeMoney();
    cout << endl;
}
 
void DoNewRun2()
{
    cout << "----------房地产公司是这样运行的----------" << endl;
    CHouse house;
    CNewHouseCorp newHouseCorp(&house);
    newHouseCorp.MakeMoney();
    cout << endl;
 
    cout << "----------山寨公司是这样运行的----------" << endl;
    CIPod ipod;
    CShanZhaiCorp shanZhaiCorp(&ipod);
    shanZhaiCorp.MakeMoney();
    cout << endl;
}
 
int main()
{
    DoNewRun1();
 
    DoNewRun2();
    return 0;
}
 </code></pre>
 
这里我们要说明的是Java中我们可以使用super.func()的方式调用基类，在C++中，我们直接this->CNewCorp::MakeMoney();来调用基类

另外，我们要深刻理解动态绑定的灵活性！

 
客户直接使用CNewHouseCorp和CShanZhaiCorp类.

####在main()函数里构造产品，然后传到这两个类里。这两个类的MakeMoney()函数，先调用基类的MakeMoney()，然后分别执行各自的逻辑。各自逻辑就是子类的具体实现（concreteimplement）

 
参考[http://www.cnblogs.com/jiese/p/3164940.html](http://www.cnblogs.com/jiese/p/3164940.html)

