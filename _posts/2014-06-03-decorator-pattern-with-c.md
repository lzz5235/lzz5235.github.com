---
layout: post
title: "Decorator Pattern with C++"
categories: [designpattern]

tags: [C/C++]
---
装饰者模式 C++实现
====================
装饰者模式就是在原来类的基础上添加额外的功能，这种方式主要是为了应对继承带来的的类膨胀。

装饰者模式比起继承更加灵活，一般来说，基类是抽象的Component还有一个具体的concreteComponent。这个模式的核心就是继承Component的Decorator类。，然后由Decorator类统一管理拓展的业务，也就是新增加的功能。


#####Decorator类中必然有一个private的Component，这个Component来执行concreteComponent的核心操作，而继承Decorator的都是负责装饰这个concreteComponent的

![](/assets/pic/decorator.png)

<pre><code>
#include <iostream>
#include <string>
using namespace std;
class Beverage
{
protected:
     string description;
public:
     virtual string getDescription() = 0;
     virtual double cost() = 0;
};
class BeverageDecorator:public Beverage
{ 
     string getDescription()
     {
      return description;
     }
};
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
class Milk:public BeverageDecorator
{
     Beverage* beverage;
public:
     Milk(Beverage *beverage)
     {
         this->beverage = beverage;
     }
     string getDescription()
     {
         return beverage->getDescription()+" Milk";
     }
     double cost()
     {
      return beverage->cost() + 0.1;
     }
};
class ConcreteBeverage:public Beverage
{
public:
     ConcreteBeverage()
     {
         description = "Espresso";
     }
     string getDescription()
     {
         return description;
     }
     double cost()
     {
         return 1.99;
     }
};
void main()
{
     //买个Espresso
     Beverage * beverage1 = new ConcreteBeverage;
     cout << beverage1->getDescription() + " $ " << beverage1->cost()<< endl;
     //买个Espresso+2份Mocha+1份Milk
     Beverage * beverage2 = new ConcreteBeverage;
     beverage2 = new Mocha(beverage2);
     beverage2 = new Mocha(beverage2);
     beverage2 = new Milk(beverage2);
     cout << beverage2->getDescription() + " $ " << beverage2->cost()<< endl;
}
</code></pre>

装饰者模式的优点就是：

> 1.Decorator与concreteComponent独立，不会相互耦合。

> 2.拓展性非常好，比如我们有father son grandson ，三层继承。通过装饰者模式，我们可以抽象为两层 father son DecoratorSon.这样很好的完成了变更。



#####缺点就是 多层继承容易带来复杂，维护性变差。需要适度使用，否则，很难维护！

######因为我用的是C++实现，所以我使用了string类型拼接，达到装饰的目的。

######在Java中我们可以使用@override重写抽象类执行的核心函数和在子类中配合新添加的业务逻辑和调用super（）方法从而调用父类函数，这样达到装饰的目的。