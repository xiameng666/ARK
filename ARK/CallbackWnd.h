#pragma once
#include "interface.h"
#include <vector>

class CallbackWnd : public ImguiWnd {
public:
    explicit CallbackWnd(Context* ctx);
    void Render(bool* p_open = nullptr) override;
    void RenderCallbackWnd();
    
private:
    // 回调类型名称映射
    const char* GetCallbackTypeName(CALLBACK_TYPE type);
    const char* GetCallbackTypeDesc(CALLBACK_TYPE type);
    
}; 