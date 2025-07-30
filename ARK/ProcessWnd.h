// ProcessWnd.h
#pragma once
#include "interface.h"

enum ProcSearchType {
    Links,  // ActiveProcessLinks
    Mem     // 内存特征
};

class ProcessWnd : public ImguiWnd {
public:
    explicit ProcessWnd(Context* ctx);
    void Render(bool* p_open = nullptr) override;

    void RenderProcessWnd();
    void RenderMemWnd(DWORD pid);
    void RenderProcessModuleWnd();         // 进程模块窗口

    void Flush(ProcSearchType type);      // 重新获取进程列表
private:


    //std::vector<PROCESSENTRY32> processList;

    MemoryEditor mem_edit;
};
