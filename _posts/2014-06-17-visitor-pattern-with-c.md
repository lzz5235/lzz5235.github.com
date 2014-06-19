---
layout: post
title: "Visitor Pattern with C++"
categories: [designpattern]

tags: [C/C++]
---
访问者模式 C++实现
==================
访问者模式，顾名思义就是使用vistor类来访问其他类的intern数据，而不改变intern的数据结构和代码。
这样可以分开数据显示和业务逻辑，使得代码更加清晰。

访问者模式的优点就是：灵活度非常高，有优秀的拓展性。
缺点就是：不依赖抽象元素，违反了依赖倒置的原则，直接依赖实现类，拓展实现类比较难。

> 访问者模式和迭代器模式有相同点又有不同点。
相同点都是访问具体的实现类。
不同点在于迭代器模式只能访问相同由base class引用来的class或者interface的数据，而访问者模式可以访问不同的class，不再依赖base class

#####总的来说 访问者模式适用范围大于迭代器模式的范围！

![](/assets/pic/visitor.png)

下面这种方式是使用不同访问者定义不同的访问方式，可以说是多个访问者。

<pre><code>
#include <iostream>
#include <string>
#include <list>
using namespace std;
 
class BaseElement;
 
class BaseVisitor
{
public:
    virtual void Visit( BaseElement *element ){};
};
 
class BaseElement
{
public:
    // Methods
    virtual void Accept( BaseVisitor *visitor ){};
};
 
 
// "ConcreteElement"
class Employee : public BaseElement
{
public:
    string name;
    double income;
    int vacationDays;
 
public :
    Employee( string name, double income,
        int vacationDays )
    {
        this->name = name;
        this->income = income;
        this->vacationDays = vacationDays;
    }
 
    void Accept( BaseVisitor *visitor )
    {
        visitor->Visit( this );
    }
};
 
class IncomeVisitor : public BaseVisitor
{
public: 
    void Visit( BaseElement *element )
    {
        Employee *employee = ((Employee*)element);
        employee->income *= 1.10;
        cout<<employee->name<<" 's new income: " << employee->income<< endl; 
    }
};
 
class VacationVisitor : public BaseVisitor
{
public :
    void Visit( BaseElement *element )
    {
        Employee *employee = ((Employee*)element);
        employee->vacationDays += 3;     
        cout<< employee->name <<" 's new vacation days: " << employee->vacationDays<< endl;
    }
};
 
class Employees
{   
private :
    list< Employee*> employees;
 
public :
 
    void Attach( Employee *employee )
    {       
        employees.push_back(employee);      
    }
 
    void Detach( Employee *employee )
    {
        employees.remove(employee);     
    }
 
    void Accept( BaseVisitor *visitor )
    {       
        for (std::list<Employee*>::iterator it=employees.begin(); it != employees.end(); ++it)
            (*it)->Accept(visitor);
    }
};
 
void main( )
{
    Employees *e = new Employees();
    e->Attach( new Employee( "Jason", 25000.0, 14 ) );
    e->Attach( new Employee( "Albter", 35000.0, 16 ) );
    e->Attach( new Employee( "lily", 45000.0, 21 ) );
 
    IncomeVisitor *v1 = new IncomeVisitor();
    VacationVisitor *v2 = new VacationVisitor();
 
    e->Accept( v1 );
    e->Accept( v2 );
}
</code></pre>

在vistor模式中，以及以前的design pattern中，我们都使用了动态绑定！
动态绑定就是在基类中定义virtual function 然后继承该base class，创建derive class时转到base class。当我们使用derive class中的function，我们只需要把base class指针指向该函数即可！

> 我们要分清函数重载与动态绑定在工程中的区别（重载只是函数类型不同而已！）

> 动态绑定中，函数的执行操作决定于请求种类，与接收者类型！

访问者模式适合于重构！比如功能集中化，UI集中化。

