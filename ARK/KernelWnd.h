#pragma once
#include "interface.h"

class KernelWnd : public ImguiWnd {
public:
    explicit KernelWnd(Context* ctx);
    void Render(bool* p_open = nullptr) override;

    const char* GetInterruptPurpose(ULONG index);
    const char* GetIDTDescriptorType(USHORT type);
private:
    void RenderGDTTable();
    void RenderShadowSSDTTable();
    void RenderIDTTable();
    void RenderSSDTTable();    
    void RenderLeftBar();

    
};
