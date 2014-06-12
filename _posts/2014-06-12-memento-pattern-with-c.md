---
layout: post
title: "Memento Pattern with C++"
categories: [designpattern]

tags: [C/C++]
---
备忘录模式C++实现
=================
备忘录模式是一种比较重要的设计模式，如果程序执行错误，我们可以恢复restore之前的状态，就好像我们在编辑word文档，需要撤销原操作，那么我们使用ctrl+Z。
在数据库操作中，这个模式用的非常多！

说白了就是在不破坏封装性的前提下，捕获一个对象的内部状态，并在该对象之外保存这个状态，这样以后就可将该对象恢复到原先保存的状态。

这个模式有三个对象：

Originator（执行业务逻辑的class）：负责创建一个备忘录Memento，用以记录当前时刻它的内部状态，并可使用备忘录恢复内部状态。Originator可根据需要决定Memento存储Originator的哪些内部状态。

Memento（备份数据的class）:负责存储Originator对象的内部状态，并可防止Originator以外的其他对象访问备忘录Memento。备忘录有两个接口，Caretaker只能看到备忘录的窄接口，它只能将备忘录传递给其他对象。Originator能够看到一个宽接口，允许它访问返回到先前状态所需的所有数据。

Caretaker（说白了就是管理备份数据的class）：负责保存好备忘录Memento,不能对备忘录的内容进行操作或检查。

#####由于备忘录模式非常灵活，往简单说可以保存单state，多state。甚至是一个保存class，往更大的说了，就是一个备份系统，每隔一段时间备份一次数据。

#####不过使用备忘录模式的时候，要考虑控制建立Memento的数量，大的object非常消耗资源，有可能产生内存泄露。

下面我先来说对于单变量的备忘录模式。
![](/assets/pic/memento.png)
<pre><code>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
 
//保存数据的class
class Memento
{
private:
    friend class Originator;
    Memento(const string& state){this->_state = state;}
    ~Memento(){}
    void SetState(const string& state){this->_state = state;}
    string GetState(){return this->_state;}
    string _state;
};
 
 
//备份数据逻辑的class
class Originator
{
public:
    Originator();
    Originator(const string& state){this->_state = state;}
    ~Originator(){}
    void RestoreToMemento(Memento* pMemento){this->_state = pMemento->GetState();}
    Memento* CreateMemento(){return new Memento(this->_state);}
    void SetState(const string& state){this->_state = state;}
    string GetState(){return this->_state;}
    void show(){cout << this->_state << endl;}
protected:
private:
    string _state;
};
 
//管理备份数据的class
class Caretaker
{
public:
    Caretaker(){}
    ~Caretaker(){}
    void SetMemento(Memento *pMemento){this->_memento = pMemento;}
    Memento* GetMemento(){return this->_memento;}
private:
    Memento* _memento;
};
int main()
{
    //初始化对象，状态为“Old”
    Originator* o = new Originator("Old");
    o->show();
 
    //建立并保存Memento
    Caretaker* pTaker = new Caretaker();
    pTaker->SetMemento(o->CreateMemento());
 
    //改变状态
    o->SetState("New");
    o->show();
 
    //恢复状态
    o->RestoreToMemento(pTaker->GetMemento());
    o->show();
 
    return 0;
}
</code></pre>
上面我们只是保存了一个state值，如果要保存一系列的值，小于6个，个人推荐直接在构造函数里传值保存！
如果是保存一个类，在Java中我们有clone（）方法，但是在C++中，我们没有现成的cloneable接口，我们无法进行深拷贝，只能借助库或者是直接实现，这里我们可以参考之前的原型模式（protype pattern）实现方式。

对于多状态备忘录模式在Java中我们可以使用BeanUtils工具类，在Java中使用BeanUtils；类以及反射的技术，进行保存，精髓就是使用hashmap来进行保存！

对于多备份备忘录，我们构造一个hashmap来存放不同时刻的memento

<pre><code>
class MultiCaretaker
{
public:
    MultiCaretaker(){}
    ~MultiCaretaker(){}
    void SetMemento(string name,Memento *pMemento){this->memmap.insert(pair<string,Memento*>(name,pMemento));}
    Memento* GetMemento(string name){return this->memmap[name];}
private:
    hash_map<string,Memento*> memmap;
};
</code></pre>
如果为了封装好一点，可以将memento放在originator中，只有originator可以调用memento！
刚才在上面做的那种单状态保存的方式，使用friend属性，其实打破了OO的开闭原则，存在一定的缺陷！

#####Java中有反射机制，C++中如何具体实现这种机制还要继续学习一下！

参考
[http://blog.csdn.net/wuzhekai1985/article/details/6672906](http://blog.csdn.net/wuzhekai1985/article/details/6672906)

[http://blog.csdn.net/ai92/article/details/454354](http://blog.csdn.net/ai92/article/details/454354)


