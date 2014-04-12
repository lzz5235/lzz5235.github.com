---
layout: post
title: "打印Frame Info"
categories: [JOS]

tags: [Kernel,JOS]

---
打印Frame Info值
================
最近在做JOS的系统实现，遇到一个小小的问题在于%.*s的用法。
<pre>
<code>
#include <cstdio>
#include <iostream>
 
int main()
{    
    char *s = "this is test example";
    int a,b;
    printf("%.*s\n", 10, s);//这里的常量10就是给*号的,你也可以用一个变量来控制宽度
    printf("%*.*s\n", 20, 10, s);//常量20控制输出所占位宽，也可以用一个变量控制
    std::cin>>a>>b; //输入15 10
    printf("%*.*s\n", a, b, s);//输出为：-----this is te
    std::cin.get();
    std::cin.ignore();//暂停程序执行
 
    return 0;
}
</code>
</pre>
esp：寄存器存放当前线程的栈顶指针
ebp：寄存器存放当前线程的栈底指针
eip：寄存器存放下一个CPU指令存放的内存地址，当CPU执行完当前的指令后，从EIP寄存器中读取下一条指令的内存地址，然后继续执行。

看汇编的时候，经常有
push ebp ; 保存当前ebp
mov ebp,esp ; EBP设为当前堆栈指针
sub esp, xxx ; 预留xxx字节给函数临时变量.
…
我们要遍历所有ebp，那就将ebp里面值取出，里面存的就是上个ebp的值！
<pre><code>
int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
    int i;
    unsigned int ebp = read_ebp();
    struct Eipdebuginfo info;
    char buffer[100];
                                                                                                                                               
    cprintf("Stack backtrace:\n");
    while(ebp!=0)
    {   
        unsigned int eip = *((unsigned int *)ebp+1);
        unsigned int args[5] = {0} ;
        for(i= 0;i<5;i++)
            args[i] = *((unsigned int *)ebp+ 2 + i); 
     
        cprintf(" ebp %08x eip %08x args %08x %08x %08x %08x %08x\n",ebp,eip,args[0],args[1],args[2],args[3],args[4]);
     
        debuginfo_eip(eip,&info);
 
        for(i=0;i<100&&i<info.eip_fn_namelen;i++)
            buffer[i] = info.eip_fn_name[i];
        buffer[i] = '\0';
 
        cprintf("      %s:%d:%.*s+%d\n",info.eip_file, info.eip_line, info.eip_fn_namelen ,info.eip_fn_name ,eip-info.eip_fn_addr);
              
        ebp = *((unsigned int *)ebp);
    }       
    return 0;
}  
</code></pre>
打印结果：
<pre><code>
Welcome to the JOS kernel monitor!
Type 'help' for a list of commands.
K> backtrace
Stack backtrace:
 ebp f010ff68 eip f01009f1 args 00000001 f010ff80 00000000 f010ffc8 f0112540
      kern/monitor.c:149:monitor 
 ebp f010ffd8 eip f01000f6 args 00000000 00001aac 00000644 00000000 00000000
      kern/init.c:43:i386_init 
 ebp f010fff8 eip f010003e args 00111021 00000000 00000000 00000000 00000000
      kern/entry.S:83:<unknown> 
</code></pre>
