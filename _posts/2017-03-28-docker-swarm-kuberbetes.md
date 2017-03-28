---
layout: post
title: "Docker 编排工具简述"
categories: linux
tags: Docker swarm mode,Kuberbetes,Mesosphere
---
编排是一个新的词汇，经过阅读才明白编排指的是容器的集群化和调度。另一类含义指的是容器管理，负责管理容器化应用和组件任务。

典型的编排工具有：Docker swarm mode、Kuberbetes和Mesosphere DCOS，这三个工具都提供相同的特性，但同时三个工具所处于的地位又不尽相同。

    1.这些工具在容器集群中提供或者调度容器，还可以启动容器。会根据需求，例如资源和部署位置，在最佳VM中启动容器。

    2. 脚本保证你把指定的配置加载到容器中。

    3. 容器管理工具跟踪和监控容器的健康，将容器维持在集群中。正常情况下，监视工具会在容器崩溃时启动一个新实例。如果服务器故障，工具会在另一台服务器上重启容器。这些工具还会运行系统健康检查、报告容器不规律行为以及VM或服务器的不正常情况。

    4.需要部署新版本的容器或者升级容器中应用时，容器管理工具自动在集群中更新你的容器或应用。如果出现问题，它们允许你回滚到正确配置的版本。

    5.容器使用服务发现来找到它们的资源。

    6.你希望容器运行在哪里？你希望每个容器分配多少CPU？所有这些需求都可以通过设置正确的容器部署策略实现。

    7.容器要能够和已有的IT管理工具兼容。

=================

###Docker Swarm mode 和 Docker Swarm 

这是两款独立的编排工具，Docker Swarm是一种较旧的独立产品，曾经用于管理Docker集群。而Swarm mode是Docker内置的集群管理器。Docker 1.12后，Swarm mode引入Docker ，成为Docker Engine的一部分。

Docker swarm mode 支持滚动更新、节点间传输层安全加密、负载均衡和简单的服务抽象。可以在多个主机之间传播容器负载，它允许你在多个主机平台上设置swarm（即群集）。

###Kubernetes

Kubernetes最初由有谷歌开发的开源容器管理工具。这个工具提供高度的互操作性，以及自我修复、自动升级回滚以及存储编排。目前负载均衡做的还不是很好，但是我们仍然需要在Kubernets基础上加入监控日志系统。

###Marathon

Marathon是为Mesosphere DC/OS和Apache Mesos设计的容器编排平台。Marathon有很多特性，包括高可用、服务发现、负载均衡。

综上所述Mesos和Kubernetes主要用于运行集群应用程序。Mesos专注于通用调度，以插件的方式提供多个不同的调度器。而Kubernetes者用来构建容器的分布式环境。对于可以结合三个工具的优缺点，来构建容器云达到更好的管理效果。

 

[https://docs.docker.com/engine/swarm/](https://docs.docker.com/engine/swarm/)

[https://kubernetes.io/](https://kubernetes.io/)

[https://github.com/mesosphere/marathon](https://github.com/mesosphere/marathon)
