---
layout: post
title: "Behavior DesignPattern Comparison"
categories: [designpattern]

tags: [C/C++]
---
行为类模式比较
===============
行为类模式包括责任链模式，命令模式，解释器模式，迭代器模式，中介者模式，备忘录模式，观察者模式，状态模式，策略模式，模板方法模式，访问者模式。这些模式都是经常要使用的。

###1.命令模式和策略模式
二者都是很重要的，只是命令模式多了一个Receiver的角色。
####策略模式主要是封装，封装一个完整的，不可拆分的原则业务，然后让这些业务独立，并可以相互替换，让行为的变化独立于拥有行为的客户，其实就是一种算法替换问题，策略模式的粒度要比命令模式的粒度大一些，也就是说对算法的更改就是全部都要更改。

####比策略模式更加抽象的就是命令模式了，但是同时业务粒度变小，可以进行更加细致的调整！


比如我们在一个demo中最初使用策略模式实现，如果进行重构，我们就要启用命令模式了！

看类图：

![](/assets/pic/1855.png)

核心代码就是下面这一块，通过动态绑定，执行我们传入类的具体函数Operate（）

<pre><code>
class context
{
public:
    context(IStrategy *isstrategy):m_strategy(isstrategy){}
    ~context(){}
    void Operate(void){m_strategy->Operate();}
private:
    IStrategy *m_strategy;
 
};
</code></pre>


####命令模式关注的是解耦，如何让请求者和执行者解耦是这个模式的要求，也就是说把请求的内容封装成一个一个的命令，由接收者执行，并且可以对命令进行多种处理，例如撤销记录什么的。多用于多命令多撤销的场合。

下面来看看命令模式的UML diagram:

![](/assets/pic/4746.png)

####调用关系就是 Invoker -> Commander -> Receiver

####Receiver是具体的执行者，在Receiver中是具体的业务执行者。对于mian（），我们只需要调用Invoker（）执行就可以了，剩下的由Commander传递到Receiver中。

 

###2.策略模式和状态模式

####二者都是通过Context来进行委托！委托就是通过动态绑定实现的。比如下面的Request()方法。Request（）方法调用Handle（）
####首先我们要明确策略模式是无状态的，我们通过传入不同策略来达到不同的行为！

####状态模式是有状态的，通过状态来改变行为，我们调用的时候传入状态即可，这里我们要阐述一下，状态模式可以有单状态多状态，状态的改变可以是单向的，也可以是双向的。

![](/assets/pic/StatePattern.png)

不同点在于应用场景不同，策略模式是平行的，可相互替换的。状态模式要求的是有状态有行为的场景，也就是一个对象必须有状态和行为，状态和行为是一一对应的。而且明显状态模式的复杂度要比策略模式高很多。

<pre><code>
class Context
{
public:
    Context(State* pState){this->_state = pState;}
    ~Context(){}
    void Request()
    {
        if(NULL != this->_state)
        {
            this->_state->Handle(this);
        }
    }
    void ChangeState(State* pState)
    {
        this->_state = pState;
    }
protected:
private:
    State* _state;
};
</code></pre>

###3.观察者模式与责任者模式

观察者模式很多都是将几个Observer插入到容器中，然后在需要的时候update，达到通知的目的。

####也就是说上下级没有关系，都是接收同样的对象，所有传递的对象都是从链首传递过来的，上一节点是没有关系的，具体怎么传播是模式内部的事，可以是线性的传播，也可以是广播，也就是说按照自己的逻辑处理就好了。


####责任链模式就有上下级关系，比如DNS解析就是这种模式，顶级DNS服务器并不与用户直接解析，而是返回给下级的DNS，层层传送给用户，也就是说与用户交互的永远是LocalDNS！