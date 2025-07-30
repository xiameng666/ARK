#pragma once
#include "Interface.h"

class HookWnd : public ImguiWnd {
public:
    explicit HookWnd(Context* ctx);
    void Render(bool* p_open = nullptr) override;
    void RenderMJHookWnd();           // ≈…«≤∫Ø ˝HookºÏ≤‚
    void RenderDeviceStackWnd();      // …Ë±∏’ª

};
