#pragma once
#include "Interface.h"

class FileWnd : public ImguiWnd {
public:
    explicit FileWnd(Context* ctx);
    void Render(bool* p_open = nullptr) override;
}; 