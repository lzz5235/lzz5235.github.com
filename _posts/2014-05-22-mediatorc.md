---
layout: post
title: "Mediator模式C++实现"
categories: [designpattern]

tags: [C/C++]
---
中介者模式 C++实现
=====================
中介者模式主要是为了解决类之间依赖关系的，有些类之间存在复杂的关系，这导致代码不具有可维护性。
中介者模式把这其中各类交换的函数统一放在mediator里面，每种colleague只需要和mediator交互就可以了。

中介者模式中，每个Colleague 维护一个 Mediator，当要进行通信时，每个具体的 Colleague 直接向 ConcreteMediator 发信息，至于信息发到哪里，则由 ConcreteMediator 来决定。

2. ConcreteColleagueA 和 ConcreteColleagueB 不必维护对各自的引用，甚至它们也不知道各个的存在。

3. 优点是，各个 Colleague 减少了耦合。

#####不过这种模式最大的缺点在于如果类之间交互特别复杂，容易造成class mediator体积巨大，这时我们要考虑多创建几个mediator来负责交互。

#####设计模式很多其实都是通过class强转成父类来产生不同的效果,比如Student -> Colleague的转换，然后通过Colleague来调用子类的函数。

#####C++中我们要通过Monitor(string n = “”):Colleage(n){};来实现子类通过父类够召唤术初始化。

#####在Java中，我们可以在构造方法中使用super（）函数，达到使用父类所有方法初始化的目的。

![](/assets/pic/03054.png)

<pre><code>
#include<iostream>
#include<string>
#include <vector>
using namespace std;

class Colleage
{
private:
    string name;
	string content;
public:
	Colleage(string n = " "):name(n){};
	void set_name(string name)
	{
		this->name = name;
	}
	string get_name()
	{
		return this->name;
	}
	void set_content(string content)
	{
		this->content = content;
	}
	string get_content()
	{
		if(content.size() != 0)
			return content;
		else return "Copy that";
	}
	virtual void talk(){};

};

class Monitor : public Colleage
{
public:
	Monitor(string n = ""):Colleage(n){};
	virtual void talk()
	{
		cout<<"Monitor "<< get_name() << " Say" << get_content()<< endl;
	}
};

class Secretary : public Colleage
{
public:
	Secretary(string n = ""):Colleage(n){};
	virtual void talk()
	{
		cout<<"Secretary "<<get_name()<<" Say"<< get_content()<< endl;
	}
};

class StudentA : public Colleage
{
public:
	StudentA(string n = ""):Colleage(n){};
	virtual void talk()
	{
		cout<<"StudentA "<<get_name()<<" Say"<< get_content()<< endl;
	}
};

class StudentB : public Colleage
{
public:
	StudentB(string n = ""):Colleage(n){};
	virtual void talk()
	{
		cout<<"StudentB "<<get_name()<<" Say"<< get_content()<< endl;
	}
};

class Mediator
{
public:
	vector<Colleage*> studentList;
	virtual void add_student(Colleage *student)
	{
		studentList.push_back(student);
	};
	virtual void notify(Colleage *student){};    
};

class QQMediator : public Mediator
{
public:
	virtual void notify(Colleage *student)
	{
		student->talk();
		for(int i = 0 ; i < studentList.size() ; ++i)
		{            
			if(student != studentList[i])
			{
				studentList[i]->talk();
			}
		}
	};	
};


int main()
{
	QQMediator mediator;
	Monitor *studentMonitor = new Monitor("studentMonitor");
	Secretary *studentSecretary = new Secretary("studentSecretary");
	StudentA *studentA = new StudentA("studentA");
	StudentB *studentB = new StudentB("studentB");        

	mediator.add_student(studentSecretary);
	mediator.add_student(studentA);
	mediator.add_student(studentB);     

	studentMonitor->set_content("It's time to end the class");
	mediator.notify(studentMonitor);   
	return 0;
}

</code></pre>

