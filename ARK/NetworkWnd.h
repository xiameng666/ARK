#pragma once
#include "Interface.h"

class NetworkWnd : public ImguiWnd
{
public:
    explicit NetworkWnd(Context* ctx);
    ~NetworkWnd();

    void Render(bool* p_open = nullptr) override;

private:
   
};
