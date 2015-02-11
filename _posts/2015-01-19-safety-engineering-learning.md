---
layout: post
title: "Safety Engineering Learning"
categories: others
tags: Safety,Engineering
---
Safety Engineering 关键概念学习
========
1.Safety is about understanding potential causality in systems with limited determinism.

    Control
    Reduce Hazard

在中文中Safety与Security 概念很类似，都是安全的含义，但是我们要分清楚，区别在于是否得到system Administrator guarantee .

Security 是在没有 guarantee保证的情况下，hacker walks into system without guarantee.

Safety 则是让系统在任何情况下under my control.

##2.Fault/Error/Failure

   * Fault is a defect within the system – Software Bugs/Hardware fault/Memory Fault
   * Error is a deviation from the required operation of system or subsystem
   * A fault may lead to an error, i.e., error is a mechanism by which the fault becomes apparent
    A system failure occurs when the system fails to perform its required function

所以总的来说，Failure对于system可能会造成灾难性后果（catastrophe）。

Functional safety is a resonable response to the increased safety needs but the first option should stay to design simple and clean systems from the start and Say No where resonable safety is not possible.

3.Fault Tolerance and Robustness

System happened fault ,system won’t walk into fails to perform its required function This is tolerance !

When system faces hazard,failure lead system walk into unsafety state while reliability make system safety (It ‘s really hard!)

Reliability Definitions The ability of an item to perform a required function, under given environmental and operational conditions and for a stated period of time

##4.Mechanisms and Policies

    Mechanisms specifies how it is to be done（与硬件软件结合相关，不能轻易改变）
    
    Policies is what is to be done(具体算法)

The separation of mechanism and policy is important to provide flexibility to a system. If the interface between mechanism and policy is well defined, the change of policy may affect only a few parameters. On the other hand, if interface between these two is vague or not well defined, it might involve much deeper change to the system.

[http://www.personal.kent.edu/~rmuhamma/OpSystems/Myos/mechanicPolicy.htm](http://www.personal.kent.edu/~rmuhamma/OpSystems/Myos/mechanicPolicy.htm)

正如kernel中的scheduler一样，这个就是Mechanisms/Policies分离的例子，Mechanisms负责切换进程实体prev，next tasks，包括context_switch，而Policies 负责根据某个策略去是选取特定的task，包括RR，CFS等等…具体对应的函数就是pick_next_task()
