// ProcessWnd.h
#pragma once
#include "interface.h"

enum ProcSearchType {
    Links,  // ActiveProcessLinks
    Mem     // �ڴ�����
};

class ProcessWnd : public ImguiWnd {
public:
    explicit ProcessWnd(Context* ctx);
    void Render(bool* p_open = nullptr) override;

    void RenderProcessWnd();
    void RenderMemWnd(DWORD pid);
    void RenderProcessModuleWnd();         // ����ģ�鴰��

    void Flush(ProcSearchType type);      // ���»�ȡ�����б�
private:


    //std::vector<PROCESSENTRY32> processList;

    MemoryEditor mem_edit;
};
