---
layout: post
title: "Common DesignPattern Comparsion"
categories: [designpattern]

tags: [C/C++]
---
常用模式比较
===============
创建类模式主要描述的是如何创建模式，行为类模式主要是关注管理对象的行为，比如委托什么的。

而结构类模式着重是建立一个软件结构，便于拓展。

####行为类模式包括责任链模式，命令模式，解释器模式，迭代器模式，中介者模式，备忘录模式，观察者模式，状态模式，策略模式，模板方法模式，访问者模式。

####结构类模式包括适配器模式，桥梁模式，组合模式，装饰模式，门面模式，享元模式和代理模式。

首先来说一下策略模式和桥梁模式的异同

1.策略模式和桥梁模式的异同

###策略模式的核心就是封装！封装行为或者是算法。而桥梁模式是在不破坏封装的情况下如何抽取抽象部分和实现部分。桥梁模式的核心就是分离！桥梁模式必然有两个核心父类。然后子类继承父类，来达到抽象与具体实现分离的目的！

![](/assets/pic/bridge.png)

比如IProduct与CNewCop就是核心父类！

###如果从protype模式开发，可以从策略模式逐渐过渡到桥梁模式！

2.门面模式与中介者模式

门面模式为复杂子系统提供统一的访问界面，定义一个facade，用户只要和这个facade交互就可以避免外部模块深入系统内部产生耦合问题。

门面模式的核心是封装和隔离。

###可以这么说：如果没有facade，子系统仍然可以运行！

中介者模式使用中介class封装同时对象交互，也就是说中介class包括了子系统的class，子系统的class交互是在中介class中进行的。

中介者模式的核心就是协调class之间的通信操作。它是更加进一步的业务封装，各类都知道media类的存在，然后使用media类协调操作！

![](/assets/pic/mediapattern.png)

3.包装模式

#####包装模式最大的特点在于类中有一些类是不干活的！这些class通过动态绑定来指定其他具体实现的class来处理。

> 典型的包装模式有：装饰模式，适配器模式，门面模式，代理模式，桥梁模式。

#####这些模式都是使用委托的方式对一个对象或者是一系列对象进行包装。

门面模式是一种粗粒度的封装，不具备任何业务逻辑！

桥梁模式在抽象层产生耦合，然后再抽象层产生拓展，以后进行需求变更就是继承抽象类即可！

代理模式不希望展示对象内部细节场景。下面的java代码可以看出，构造函数中传入真实的实际操作类，然后在代理中使用该操作类达到代理的目的！

<pre><code>
public class Assistant implements Leader{  
   
    private Leader leader ;  
       
       
    public Assistant(Leader leader) {  
        super();  
        this.leader = leader;  
    }  
   
   
    @Override  
    public void sign() {  
        System.out.println("递给领导");  
        leader.sign();  
        System.out.println("装入袋子，送出");  
    }  
       
   
}  
</code></pre>

装饰者模式就是在原来类的基础上添加额外的功能，这种方式主要是为了应对继承带来的的类膨胀。

<pre><code>
class Mocha:public BeverageDecorator
{
     Beverage* beverage;
public:
     Mocha(Beverage *beverage)
     {
         this->beverage = beverage;
     }
     string getDescription()
     {
         return beverage->getDescription()+" Mocha";
     }
     double cost()
     {
         return beverage->cost() + 0.2;
     }
};
</code></pre>

####在装饰完一个类时，使用string getDescription()去递归调用return beverage->getDescription()+” Mocha”;然后从内部再层层返回出getDescription和cost。

 


