#pragma once
#include "Interface.h"
#include "ProcessWnd.h"
#include "ModuleWnd.h"
#include "KernelWnd.h"
#include "CallbackWnd.h"              
#include "LogWnd.h"
#include "MenuBar.h"

class App
{
private:
    Context ctx_;
    ProcessWnd processWnd_;
    ModuleWnd moduleWnd_;
    KernelWnd kernelWnd_;
    CallbackWnd callbackWnd_;          
    LogWnd logWnd_;
    MenuBar menuBar_;

public:
    App();
    ~App();
    void Render();
    void SetDockingWnd(bool* p_open = nullptr);
};


