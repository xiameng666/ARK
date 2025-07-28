#pragma once
#include "Interface.h"

class HookWnd : public ImguiWnd {
public:
    explicit HookWnd(Context* ctx);
    void Render(bool* p_open = nullptr) override;
    void RenderMJHookWnd();           // 派遣函数Hook检测UI
    void RenderDeviceStackWnd();      // 设备栈分析UI

};
