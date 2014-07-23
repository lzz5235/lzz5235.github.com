---
layout: post
title: "Construction DesignPattern Comparison"
categories: [designpattern]

tags: [C/C++]
---
结构类模式比较
===============
结构类模式包括适配器模式，桥梁模式，组合模式，装饰模式，门面模式，享元模式和代理模式。

首先我们来比较一下代理模式和装饰模式。

####代理模式和装饰模式其实很类似，装饰模式就是一种特殊的代理模式，两者都有相同的接口，不同点在于代理模式对于代理过程的控制，而装饰模式则是对类的功能加强和减弱。

咱们先来看代理模式的类图

![](/assets/pic/55800.png)

首先实际的class与proxy class都要继承同一个interface 这里就是IGamePlayer

GamePlayer是真正的执行者，而GamePlayerProxy是控制GamePlayer执行的。

相比代理模式，装饰者模式则是在这个代理类中的方法添加新的功能，到达装饰的目的，也就是添加删除类的功能！

看看装饰者模式的类图：

![](/assets/pic/decorator.png)

我们在getDescription（）方法中进行对类功能的增加删减达到装饰的目的，对于修饰者，我们就是继承BeverageDecorator class达到修饰的目的。

 

装饰模式与适配器模式从外面看差别很大，但是也有相似的地方。

#####装饰模式包装的是自己的兄弟类，隶属于一个家族，适配器模式则装修非血缘关系类，把一个非本家族的对象伪装成本家族的对象，注意是伪装。

看看适配器模式的类图：

![](/assets/pic/adapterPattern.png)

<pre><code>
public class UglyDuckling extends WhiteSwan implements Duck
{
     .......
     public void desBehavior()
     {
          system.out.println("swimming");
          super.fly();
     }
}
</code></pre>
我们可以看出在deBehavior（）中我们定义了与Duckling不同的函数，实现了不同的功能！

现在来说说这两种模式的不同之处：

       1、意图不同，装饰模式关注的是加强对象的功能，而适配器模式关注的是转换

       2、施与对象不同：装饰模式装饰的对象必须是自己的同宗，而适配器模式则必须是两个不同的对象

       3、场景不同：装饰模式在任何情况下都可以用，而适配器模式是一种补救措施

       4、扩充性不同：装饰模式容易扩充，而适配器模式建立容易，拆除难
