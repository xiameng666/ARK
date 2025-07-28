# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

# 项目说明
这是一个Windows内核驱动ARK项目，包含Ring-0内核驱动(NTDriver)和Ring-3用户模式GUI应用(ARK)两个主要组件，用于系统监控和反Rootkit分析。

# 身份设置
用户是正在学习windows内核的学生，你除了生产代码，更重要的是整理思维链路，数据流动，告诉用户为什么这么做。

当完成一段功能后，自动生成对应的md笔记，保存用户不懂的知识

# 代码架构

## 项目结构
```
NTDriver/          # 内核驱动组件 (Ring-0)
├── driver.cpp/h   # 主驱动入口点和IRP调度
├── process.cpp/h  # 进程枚举和管理
├── module.cpp/h   # 模块枚举
├── callback.cpp/h # 回调机制分析
└── ssdt.cpp/h     # 系统服务描述符表分析

ARK/               # 用户模式GUI应用 (Ring-3)  
├── main.cpp       # DirectX 9应用入口
├── App.cpp/h      # 主应用类
├── ProcessWnd.h   # 进程窗口UI
├── ModuleWnd.h    # 模块窗口UI
├── CallbackWnd.h  # 回调窗口UI
└── ArkR3.cpp/h    # Ring-3驱动通信层

include/proto.h    # 驱动与应用通信协议
```

## 通信机制
- 设备名称: `\\Device\\ADriver1`
- 符号链接: `\\DosDevices\\ADriver1`
- 通信方式: DeviceIoControl + 自定义IOCTL码
- 主要数据结构: PROCESS_INFO, MODULE_INFO, CALLBACK_INFO, SSDT_INFO

## 核心功能
1. **进程管理**: EPROCESS遍历、进程信息提取、进程终止
2. **模块分析**: 全局系统模块、进程模块枚举
3. **回调检测**: 进程、线程、镜像、注册表、蓝屏、关机回调
4. **SSDT分析**: 系统调用表枚举、Hook检测
5. **内存操作**: 内核内存读写、GDT分析

# 构建说明

## 必需工具
- Visual Studio 2019/2022
- Windows Driver Kit (WDK) 10
- Windows SDK
- 测试签名模式(开发期间)

## 构建步骤
```cmd
# 在Visual Studio中打开ARK_x64.sln
# 先构建ADriver1(内核驱动)，再构建ArkUserMode(GUI应用)
# 对驱动进行签名用于部署
```

# 代码风格
- 遵循用户的编程风格
- 重点解释Windows内核概念和数据流动

# 目前待做的项目

1. 完善其他type的回调函数数组遍历
2. 如何判断驱动对象被驱动过滤也就是"hook了" - 枚举出驱动的派遣函数然后查看函数指针是否在驱动的模块内吗？

# 学习要点

## Windows内核概念
- EPROCESS/ETHREAD结构体遍历
- PEB_LDR_DATA模块链表
- 回调机制原理(PspCreateProcessNotifyRoutine等)
- SSDT Hook检测原理
- 内核对象管理

## 驱动开发要点
- WDM驱动模型
- IRQL级别管理
- 内核内存管理
- 异常处理机制
- 驱动签名要求