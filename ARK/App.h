#pragma once
#include"Interface.h"
#include "LogWnd.h"
#include "KernelWnd.h"
#include "MenuBar.h"
#include "ProcessWnd.h"
#include "ModuleWnd.h"
/*
#include "ProcessWnd.h"
#include "ModuleWnd.h"
#include "RegeditWnd.h"
#include "FileWnd.h"
#include "SSDTHookWnd.h"
#include "MenuBar.h"

*/
// 全局进程事件数据
extern std::vector<PROCESS_EVENT> g_ProcessEvents;

// PDB路径设置函数声明
bool SetPdbDownloadPathToDesktop();

class App {
public:
    App();
    ~App();

    void Render();
    void SetDockingWnd(bool* p_open);

    Context ctx_;
    LogWnd logWnd_;
    KernelWnd kernelWnd_;
    MenuBar menuBar_;
    ProcessWnd processWnd_;
    ModuleWnd moduleWnd_;
    /*
    LogWnd logWnd_;
    MenuBar menuBar_;
    ProcessWnd processWnd_;
    ModuleWnd moduleWnd_;
    KernelWnd kernelWnd_;
    RegeditWnd regeditWnd_;
    FileWnd fileWnd_;
    SSDTHookWnd ssdtHookWnd_;
    */
};


