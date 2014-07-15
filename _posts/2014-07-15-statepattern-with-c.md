---
layout: post
title: "StatePattern with C++"
categories: [designpattern]

tags: [C/C++]
---
状态模式 C++实现
=================
状态模式是一种封装模式，也就是说当一个对象的内在状态改变时允许改变其行为，这个对象看起来像是改变了其类。

###它有两种使用情况：（1）一个对象的行为取决于它的状态, 并且它必须在运行时刻根据状态改变它的行为。（2）一个操作中含有庞大的多分支的条件语句，且这些分支依赖于该对象的状态。如果if-else过于庞大，那么状态模式将是很好的选择。

###之前我的文章都见过这种方式，将具体的实现class都委托给一个封装类来执行，这种方式非常普遍！比如在访问者模式中，通过

<pre><code>
void Accept( BaseVisitor *visitor )
{
   visitor->Visit( this );
}
</code></pre>

来委托给visitor 执行concrete class！
这种模式适合在权限设计，条件分子判断设计中使用，当然了，也容易造成concrete class 膨胀！

首先，我们要先说单状态的状态模式，这种模式，我们不需要使用if – else语句就可以通过状态切换，达到行为切换！

状态模式的优点在于遵循了很好的开闭原则和单一职责原则，只要修改状态，我们修改一个子类就好了。

但是存在的问题在于如果状态过多，那就会导致子类过多，发生类膨胀，所以在使用这个模式的时候要权衡好状态模式的利弊！

![](/assets/pic/StatePattern.png)

<pre><code>
#include <iostream>
#include <string>
using namespace std;
 
class Context;
class State
{
public:
    virtual void Handle(Context* pContext)=0;
    ~State(){}
protected:
    State(){}
private:
};
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
class ConcreteStateA;
class ConcreteStateC : public State
{
public:
    ConcreteStateC(){}
    ~ConcreteStateC(){}
    virtual void Handle(Context* pContext)
    {
        cout << "ConcreteStateC" << endl;
        pContext->ChangeState(new ConcreteStateA());
    }
protected:
private:
};
class ConcreteStateB : public State
{
public:
    ConcreteStateB(){}
    ~ConcreteStateB(){}
    virtual void Handle(Context* pContext)
    {
        cout << "ConcreteStateB" << endl;
        pContext->ChangeState(new ConcreteStateC());
    }
protected:
private:
};
class ConcreteStateA : public State
{
public:
    ConcreteStateA(){}
    ~ConcreteStateA(){}
    virtual void Handle(Context* pContext)
    {
        cout << "ConcreteStateA" << endl;
        pContext->ChangeState(new ConcreteStateB());
        //execute and change status
    }
protected:
private:
};
 
int main()
{
    State* pState = new ConcreteStateA();
    Context* pContext = new Context(pState);
    pContext->Request();
    pContext->Request();
    pContext->Request();
    pContext->Request();
    pContext->Request();
    return 0;
}
</code></pre>

接下来，多状态切换，我们使用switch case来判断。

#####另外如果业务需求某项业务有多个状态，通常都是一些枚举常量，状态的变化都是依靠大量的多分支判断语句来实现，此时应该考虑将每一种业务状态定义为一个State的子类。这样这些对象就可以不依赖于其他对象儿独立变化了。

![](/assets/pic/multiState.png)

<pre><code>
#include <iostream>
#include <string>
using namespace std;
 
class CContext;
class COpenningState;
class CCloseingState;
class CRunningState;
class CStoppingState;
 
COpenningState* CContext::pOpenningState = NULL;
CCloseingState* CContext::pCloseingState = NULL;
CRunningState* CContext::pRunningState = NULL;
CStoppingState* CContext::pStoppingState = NULL;
 
class CLiftState
{
public:
    CLiftState(void){}
    virtual ~CLiftState(void){}
    void SetContext(CContext *pContext){this->m_pContext = pContext;}
    virtual void Open() = 0;
    virtual void Close() = 0;
    virtual void Run() = 0;
    virtual void Stop() = 0;
protected:
    CContext *m_pContext;
};
 
 
 
class CCloseingState :  public CLiftState
{
public:
    CCloseingState(void){}
    ~CCloseingState(void){}
    void Open()
    {
        this->CLiftState::m_pContext->SetLiftState(CContext::pOpenningState);
        this->CLiftState::m_pContext->GetLiftState()->Open();
    }
    void Close(){cout << "Close...." << endl;}
    void Run()
    {
        this->CLiftState::m_pContext->SetLiftState(CContext::pRunningState);
        this->CLiftState::m_pContext->GetLiftState()->Run();
    }
    void Stop()
    {
        this->CLiftState::m_pContext->SetLiftState(CContext::pStoppingState);
        this->CLiftState::m_pContext->GetLiftState()->Stop();
    }
};
class COpenningState : public CLiftState
{
public:
    COpenningState(void){}
    ~COpenningState(void){}
    void Open(){cout << "Open...." << endl;}
    void Close()
    {
        this->CLiftState::m_pContext->SetLiftState(CContext::pCloseingState);
        this->CLiftState::m_pContext->GetLiftState()->Close();
    }
    void Run(){ }
    void Stop(){}
};
class CRunningState : public CLiftState
{
public:
    CRunningState(void){}
    ~CRunningState(void){}
    void Open(){}
    void Close(){}
    void Run(){cout<<"Run...."<< endl;}
    void Stop()
    {
        this->CLiftState::m_pContext->SetLiftState(CContext::pStoppingState);
        this->CLiftState::m_pContext->GetLiftState()->Stop();
    }
};
class CStoppingState : public CLiftState
{
public:
    CStoppingState(void){}
    ~CStoppingState(void){}
    void Open()
    {
        this->CLiftState::m_pContext->SetLiftState(CContext::pOpenningState);
        this->CLiftState::m_pContext->GetLiftState()->Open();
    }
    void Close(){}
    void Run()
    {
        this->CLiftState::m_pContext->SetLiftState(CContext::pRunningState);
        this->CLiftState::m_pContext->GetLiftState()->Run();
    }
    void Stop(){cout << "Stop...." << endl;}
};
class CContext
{
public:
    CContext(void)
    {
        m_pLiftState = NULL;
        pOpenningState = new COpenningState();
        pCloseingState = new CCloseingState();
        pRunningState = new CRunningState();
        pStoppingState = new CStoppingState();
    }
    ~CContext(void)
    {
        delete pOpenningState;
        pOpenningState = NULL;
        delete pCloseingState;
        pCloseingState = NULL;
        delete pRunningState;
        pRunningState = NULL;
        delete pStoppingState;
        pStoppingState = NULL;
    }
 
    CLiftState * GetLiftState(){return m_pLiftState;}
    void SetLiftState(CLiftState *pLiftState)
    {
        this->m_pLiftState = pLiftState;
        this->m_pLiftState->SetContext(this);
    }
    void Open(){this->m_pLiftState->Open();}
    void Close(){this->m_pLiftState->Close();}
    void Run(){this->m_pLiftState->Run();}
    void Stop(){this->m_pLiftState->Stop();}
 
    static COpenningState *pOpenningState;
    static CCloseingState *pCloseingState;
    static CRunningState *pRunningState;
    static CStoppingState *pStoppingState;
 
private:
    CLiftState *m_pLiftState;
};
int main()
{
    CContext context;
    CCloseingState closeingState;
    context.SetLiftState(&closeingState);
    context.Close();
    context.Open();
    context.Run();
    context.Stop();
}

</code></pre>

####状态模式实现了非常好的封装，对外屏蔽了状态切换，状态的切换就是类的切换！


