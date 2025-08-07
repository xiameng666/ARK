#pragma once
#include "interface.h"
#include <vector>

class ModuleWnd : public ImguiWnd {
public:
    explicit ModuleWnd(Context* ctx);
    void Render(bool* p_open = nullptr) override;
    void RenderDriverObject();
    void RenderKernelModule();
    
    void RenderModuleTable(const std::vector<MODULE_INFO>& moduleList, const char* windowTitle = u8"Ä£¿éÁÐ±í");
    
private:
    
}; 
