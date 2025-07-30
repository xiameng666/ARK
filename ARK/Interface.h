#pragma once
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include"imgui_momory_editor.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <GL/GL.h>
#include <tchar.h>

#include "../include/proto.h"
#include <string>
#include "ArkR3.h"
#include <map>
#include <functional>
#include <vector>
#include <chrono>
#include <imgui_internal.h>
#include <algorithm>

#define COLOR_RED       ImVec4(1.0f, 0.0f, 0.0f, 1.0f)    // ��ɫ
#define COLOR_GREEN     ImVec4(0.0f, 1.0f, 0.0f, 1.0f)    // ��ɫ
#define COLOR_YELLOW    ImVec4(1.0f, 1.0f, 0.0f, 1.0f)    // ��ɫ
#define COLOR_GRAY      ImVec4(0.8f, 0.8f, 0.8f, 1.0f)    // ��ɫ
#define COLOR_LIGHT_GRAY ImVec4(0.5f, 0.5f, 0.5f, 1.0f)   // ǳ��
#define COLOR_WHITE     ImVec4(1.0f, 1.0f, 1.0f, 1.0f)    // ��ɫ
#define COLOR_CYAN      ImVec4(0.0f, 0.8f, 1.0f, 1.0f)    // ��ɫ
#define COLOR_ORANGE    ImVec4(1.0f, 0.6f, 0.0f, 1.0f)    // ��ɫ
#define COLOR_PURPLE    ImVec4(1.0f, 0.0f, 1.0f, 1.0f)    // ��ɫ
#define COLOR_INFO      ImVec4(0.7f, 0.7f, 0.7f, 1.0f)    // ��Ϣ��ɫ

// ������ͼ
#define VIEW_ITEM(text, view) \
    if (ImGui::Selectable(text, ctx_->currentView == view)) { \
        ctx_->currentView = view; \
    }

// ���������
#define NAV_SECTION(group_name, ...) \
    if (ImGui::TreeNodeEx(group_name, ImGuiTreeNodeFlags_DefaultOpen)) { \
        __VA_ARGS__ \
        ImGui::TreePop(); \
    }

enum SubView {
    GDT,
    IDT,
    SSDT,
    SSDTHOOK,
    PROCESS,
    MEM,
    MODULE,
    PROCESS_MODULE,
    VIEWCALLBACK          
};


struct Context {
    ArkR3 arkR3;

    bool show_process_wnd = true;
    bool show_module_wnd = true;
    bool show_regedit_wnd = true;
    bool show_menu_bar = true;
    bool show_file_wnd = true;
    bool show_ssdthook_wnd = true;
    bool show_log_wnd = true;
    bool show_kernel_wnd = true;
    bool show_callback_wnd = true;
    bool show_hook_wnd = true;
    bool show_network_wnd = true;

    bool showMemoryWindow = false;


    // UI���ݴ洢
    std::vector<PROCESS_INFO> processUiVec;           // �����б�����
    std::vector<MODULE_INFO> globalModuleUiVec;       // ȫ��ģ������
    std::vector<MODULE_INFO> processModuleUiVec;      // ����ģ������
    std::vector<SSDT_INFO> ssdtUiVec_;                // SSDT����
    std::vector<CALLBACK_INFO> callbackUiVec;         // �ص�����
    std::vector<DISPATCH_HOOK_INFO> dispatchHookUiVec;// ��ǲ������hook����
    std::vector<DEVICE_STACK_INFO> deviceStackUiVec;  // �豸ջ��attach����

    // ����ģ�鴰��״̬
    bool showProcessModuleWnd = false;
    DWORD moduleTargetPid = 0;
    char moduleTargetProcessName[64] = "";

    //���̴���״̬
    char processIdText_[16] = "1234";   // ����ID�����
    char addressText_[16] = "00400000"; // ��ַ�����
    char sizeText_[16] = "256";         // ��С�����
    DWORD targetPid_ = 0;

    // SSDT���״̬
    bool ssdtLoaded_ = false;
    int selectedSSDTIndex_ = -1;
    char ssdtSearchFilter_[256] = "";

    // �ص����״̬
    bool callbackLoaded_ = false;
    int selectedCallbackIndex_ = -1;
    int selectedCallbackType_ = 0;                    // ��ǰѡ��Ļص�����
    char callbackSearchFilter_[256] = "";            // �ص�����������
    
    SubView currentView;
};


/*
- ImGui::IsWindowFocused() - �����Ƿ��н���
- ImGui::IsWindowHovered() - ����Ƿ���ͣ�ڴ�����
- ImGui::IsWindowAppearing() - �����Ƿ�������ʾ
*/
#define ActiveFlushWnd isAutoFlush && IsWindowVisible() 

class ImguiWnd {
protected:
    Context* ctx_;  
    std::map<SubView, std::function<void()>> viewRenderers_;//˵ʵ��ûɶ�� �Ҹ�ʱ��ɾ��

public:
    explicit ImguiWnd(Context* ctx) : ctx_(ctx) {}
    virtual void Render(bool* p_open = nullptr) = 0;
    virtual ~ImguiWnd() = default;

    //�����Զ�ˢ�µ�
    std::chrono::steady_clock::time_point lastFlushTime;
    bool isAutoFlush = true;
    int flushSecond = 4;

    void SetAutoFlush(bool enable) { isAutoFlush = enable; }
    void SetFlushSeconds(int seconds) { flushSecond = seconds; }

    bool IsWindowVisible() {
        return !ImGui::IsWindowCollapsed() &&
            ImGui::GetCurrentWindow() &&
            !ImGui::GetCurrentWindow()->Hidden;
    }

};



