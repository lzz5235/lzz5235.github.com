---
layout: post
title: "Protype_IteratorC++实现"
categories: [DesignPattern]

tags: [C/C++]



---
原型模式 迭代器模式 C++实现
============================
原型模式其实非常简单，难易程度其实就是单例模式与迭代器模式差不多。
在Java中我们可以有很多的接口使用，在这里，我们也许会用到Cloneable接口，然后在复制的类里面实现clone方法。

#####调用这个clone（）就等于完成了类的复制，而不是通过new来创建。Clone（）方法是在内存中进行拷贝，所以不会调用构造函数！

final与clone不共融，final使用后，就意味着无法进行重写，而调用clone（）必须重写！

我们知道在OO语言中，有深拷贝与浅拷贝之分。浅拷贝只拷贝类对象，类里面的数组都共享一份，这期中就有潜在的风险。而深拷贝则是完全将对象拷贝两份，也就是说内存中含有两份一模一样的对象！

#####扯完Java，我来用C++实现原型模式，C++中没有什么clone接口，所以我们完全要手动来做。

> 这里有个小插曲：
因为我是在vs2012中编译的。目前vs2012已经把strcpy函数禁用掉，因为存在不安全性。

![](/assets/pic/70658.png)

正如之前爆出的heartbleed漏洞，也是未对字符串赋值进行检查导致，所以我们这里使用strcpy_s()不过这样确实丧失了移植性。需要注意一下。
如果我们不需要这种提醒，可以在设置中把预编译检查关闭！

![](/assets/pic/71649.png)

<pre><code>
#include<iostream>
#include<string>
using namespace std;
 
 
class Mail
{
public:
    Mail(){};
    virtual Mail* Clone(){return NULL;}
    virtual void show() =0;
    char *getstr(){return name;}
protected:
    char* name;
};
 
class MailA :public Mail
{
public:
    MailA(char *str)
    {
        if(str==NULL){      name = new char[10];    name[9]='\0';}
        else{name = new char[strlen(str)+1]; strcpy_s(name,10,str);}
    }
    ~MailA(){delete name;}
    MailA(Mail &r){     name = new char[strlen(r.getstr())+1]; strcpy_s(name,strlen(r.getstr())+1,r.getstr());  }
    Mail* Clone(){return (Mail *)new MailA(*this);}
    void show(){cout << name << endl;}
};
 
class MailB :public Mail
{
public:
    MailB(char *str)
    {
        if(str==NULL){      name = new char[10];    name[9]='\0';}
        else{name = new char[strlen(str)+1]; strcpy_s(name,10,str);}
    }
    ~MailB(){delete name;}
    MailB(Mail &r){     name = new char[strlen(r.getstr())+1]; strcpy_s(name,strlen(r.getstr())+1,r.getstr());  }
    Mail* Clone(){return (Mail *)new MailB(*this);}
    void show(){cout << name << endl;}
};
 
int main()
{
    Mail *m1 = new MailA("A");
    Mail *m2 = new MailB("B");
    Mail *m3 = m1->Clone();
    Mail *m4 = m2->Clone();
    m1->show(); m2->show(); //删除m1,m2  
    delete m1; delete m2;     
    m1 = m2 = NULL;  
 
    //深拷贝所以对m3,m4无影响  
    m3->show(); m4->show();  
    delete m3; delete m4;  
    m3 = m4 = NULL;  
}
</code></pre>

#####迭代器模式如STL，通过容器来管理复杂的数据结构，类等。然后通过iteration来迭代，俗称迭代器模式。
在Java中有现成的迭代器来使用，在C++中，我们需要自己实现这种接口iterator。

![](/assets/pic/3617.png)

<pre><code>
#include<iostream>
#include<string>
#include <vector>
using namespace std;
 
template <class T>
class Iterator
{
public:
    virtual void first()=0;
    virtual void next()=0;
    virtual T* currentItem()=0;
    virtual bool isEnd()=0;
    virtual ~Iterator(){}
};
 
template <class T>
class RealSet;
 
template <class T>
class RealIterator:public Iterator <T>
{
private:
    RealSet<T> *rset;
    int cur_ptr;
public:
    RealIterator(RealSet<T> *a):rset(a),cur_ptr(0){}
    void first(){cur_ptr=0;}
    void next(){if(cur_ptr < rset->getlen() ) cur_ptr++;}
    T* currentItem()
    {
        if(cur_ptr <rset->getlen())   return &(*rset)[cur_ptr];
        else return NULL;
    }
    bool isEnd()
    {
        if(cur_ptr >=rset->getlen())return false;
        return true;
    }
};
 
template <class T>
class Set
{
public:
    virtual Iterator<T>* createIterator()=0;
    virtual void push(T a)=0;
    virtual ~Set(){}
};
 
template <class T>
class RealSet:public Set<T>
{
private:
    vector<T> data;
public:
    RealSet(){}
    ~RealSet(){}
    virtual Iterator<T>* createIterator(){ return new RealIterator<T>(this);}
    virtual T& operator[](int index){return data[index];}
    int getlen(){return data.size();}
    void push(T a)
    {
        data.push_back(a);
    }
};
 
int main()
{
    Set<double> *set = new RealSet<double>();
    Iterator<double> *iter = set->createIterator();
    set->push(3.22);
    set->push(1.86);
    set->push(0.88);
 
    for(iter->first();iter->isEnd();iter->next())
    {
        cout << *iter->currentItem() << endl;
    }
    delete iter;
    delete set;
    return 0;
}
</code></pre>

######确实这种template编程很难，我调试了一下好久才找到问题所在。。。。。。
