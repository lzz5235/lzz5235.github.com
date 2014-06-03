---
layout: post
title: "Commander Pattern and Chain of Responsibility Pattern with C++"
categories: [DesignPattern]

tags: [C/C++]
---
命令模式 责任链模式 C++实现
=============================
命令模式是一种高内聚的设计模式，这种模式主要有三类角色：
1.Receiver ———–就是实际要具体执行的具体函数
2.Commander ———-执行命令的操作在这里
3.Invoker ———-他是一个调用者的角色，接受Commander然后调用Receiver
这样使得调用方和执行方二者分离开来，实现了解耦。

有时候Receiver与Commander会合并，但在大型的项目，三者还是分离开来的。 

######这种模式主要是解耦类与类之间复杂的关系。

其实调用树就是 Invoker -> Commander -> Receiver

![](/assets/pic/4746.png)

<pre><code>
#include <iostream>
using namespace std;
  
#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }
  
class Receiver
{
public:
     void Action()
     {
          cout <<"Receiver->Action"<< endl;
     }
};
  
class Command
{
public:
     virtual void Execute() = 0;
};
  
class ConcreteCommand : public Command
{
public:
     ConcreteCommand(Receiver *pReceiver) : m_pReceiver(pReceiver){}
     void Execute()
     {
          m_pReceiver->Action();
     }
private:
     Receiver *m_pReceiver;
};
  
class Invoker
{
public:
     Invoker(Command *pCommand) : m_pCommand(pCommand){}
     void Invoke()
     {
          m_pCommand->Execute();
     }
private:
     Command *m_pCommand;
};
  
int main()
{
     Receiver *pReceiver = new Receiver();
     Command *pCommand = new ConcreteCommand(pReceiver);
     Invoker *pInvoker = new Invoker(pCommand);
     pInvoker->Invoke();
     SAFE_DELETE(pInvoker);
     SAFE_DELETE(pCommand);
     SAFE_DELETE(pReceiver);
     return 0;
}
</code></pre>

> ######有时候，command可以和receive合在一起。这样可以减少暴露在client中的类，使得更加解耦。我们在上面可以加入setCommand（）方法达到一样的效果。但是下面这种方式，比起通用类图，使得类之前联系减少，效果更好！

<pre><code>
class Receiver
{
public:
     void Action()
     {
          cout<<"Receiver->Action"<< endl;
     }
};
  
class Command
{
public:
     virtual void Execute() = 0;
};
  
class ConcreteCommand : public Command
{
public:
     ConcreteCommand(){m_pReceiver = new Receiver();}
     void Execute()
     {
          m_pReceiver->Action();
     }
private:
     Receiver *m_pReceiver;
};
  
class Invoker
{
public:
     Invoker(Command *pCommand) : m_pCommand(pCommand){}
     void Invoke()
     {
          m_pCommand->Execute();
     }
private:
     Command *m_pCommand;
};
  
int main()
{
     Command *pCommand = new ConcreteCommand();
     Invoker *pInvoker = new Invoker(pCommand);
     pInvoker->Invoke();
     SAFE_DELETE(pInvoker);
     SAFE_DELETE(pCommand);
     return 0;
}
</code></pre>

责任链模式非常高效。
他把请求和处理分开，请求者不用担心是谁处理的，处理者如果不能处理该请求，则这个处理请求会传递给下一个处理者尝试处理。这个原理主要是通过链表对象遍历做到的。
不过需要注意的是链表对象的遍历可能导致请求一直在寻找处理者，导致性能低下。

总的来说都是父类实现请求传递，在子类中实现请求的·处理。

但是要注意：

> 1.一个请求到链的最后可能也没有处理，所以一定要配置得当.

> 2.责任链模式并不创建责任链。责任链的创建必须由系统的其它部分创建出来。

> 3.责任链模式降低了请求的发送端和接收端之间的耦合，使多个对象都有机会处理这个请求。一个链可以是一条线，一个树，也可以是一个环。

![](/assets/pic/ClassDiagram.png)

<pre><code>
#include <iostream>
#include <string>
using namespace std;
 
class Photo 
{
public:
    Photo(string s) : mTitle(s) 
    {
    cout << "Processing " << mTitle << " ...\n";
    }
 
private:
    string mTitle;
};
 
class PhotoProcessor
{
public:
    PhotoProcessor() : mNextProcessor(0){ }
 
public:
    void process(Photo &p) 
    {
        processImplementation(p);
        if (mNextProcessor != 0) 
            mNextProcessor->process(p);
    }
 
    virtual ~PhotoProcessor() { }
 
    void setNextProcessor(PhotoProcessor *p) {
        mNextProcessor = p;
    }
 
protected:
    virtual void processImplementation(Photo &a) = 0;
 
private:
    PhotoProcessor *mNextProcessor;
};
 
class Scale : public PhotoProcessor
{
public:
    enum SCALE { S50, S100, S200, S300, S500 };
    Scale(SCALE s) : mSCALE(s) { }
 
private:
    void processImplementation(Photo &a) {
        cout << "Scaling photo\n";
    }
 
    SCALE mSCALE;
};
 
class RedEye : public PhotoProcessor
{
private:
    void processImplementation(Photo &a) {
        cout << "Removing red eye\n";
    }
};
 
class Filter : public PhotoProcessor
{
private:
    void processImplementation(Photo &a) {
        cout << "Applying filters\n";
    }
};
 
class ColorMatch : public PhotoProcessor
{
private:
    void processImplementation(Photo &a)
    {
        cout << "Matching colors\n";
    }
};
 
void processPhoto(Photo &photo)
{   
    ColorMatch match;
    RedEye eye;
    Filter filter;
    Scale scale(Scale::S200);
    scale.setNextProcessor(&eye);
    eye.setNextProcessor(&match);
    match.setNextProcessor(&filter);
// scale -> eye -> match -> filter
//也就是说scale eye match filter都保存着下一个class，这样就形成了chain of responsibility
    scale.process(photo);
}
 
int main()
{
    Photo *p = new Photo("lzz");
    processPhoto(*p);
    return 0;
}
</code></pre>