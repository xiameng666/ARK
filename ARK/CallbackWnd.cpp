#include "CallbackWnd.h"
#include <algorithm>

CallbackWnd::CallbackWnd(Context* ctx) 
    : ImguiWnd(ctx)
{
    viewRenderers_[VIEWCALLBACK] = [this]() { RenderCallbackWnd(); };
}

void CallbackWnd::Render(bool* p_open)
{
    ImGui::Begin(u8"系统回调", p_open);
    
    RenderCallbackWnd();
    
    ImGui::End();
}

void CallbackWnd::RenderCallbackWnd() {
    
    // 回调类型选择区域 - 使用RadioButton
    ImGui::Text(u8"回调类型:");
    ImGui::Separator();
    
    // 左侧回调类型选择区域
    ImGui::BeginChild("callback_types", ImVec2(200, 0), true);
    
    if (ImGui::RadioButton(u8"进程创建回调", ctx_->selectedCallbackType_ == TypeCreateProcess)) {
        ctx_->selectedCallbackType_ = TypeCreateProcess;
        ctx_->callbackLoaded_ = false;
        ctx_->callbackUiVec.clear();
    }
    
    if (ImGui::RadioButton(u8"进程创建回调(扩展)", ctx_->selectedCallbackType_ == TypeCreateProcessEx)) {
        ctx_->selectedCallbackType_ = TypeCreateProcessEx;
        ctx_->callbackLoaded_ = false;
        ctx_->callbackUiVec.clear();
    }
    
    if (ImGui::RadioButton(u8"线程创建回调", ctx_->selectedCallbackType_ == TypeCreateThread)) {
        ctx_->selectedCallbackType_ = TypeCreateThread;
        ctx_->callbackLoaded_ = false;
        ctx_->callbackUiVec.clear();
    }
    
    if (ImGui::RadioButton(u8"映像加载回调", ctx_->selectedCallbackType_ == TypeLoadImage)) {
        ctx_->selectedCallbackType_ = TypeLoadImage;
        ctx_->callbackLoaded_ = false;
        ctx_->callbackUiVec.clear();
    }
    
    if (ImGui::RadioButton(u8"注册表回调", ctx_->selectedCallbackType_ == TypeRegistry)) {
        ctx_->selectedCallbackType_ = TypeRegistry;
        ctx_->callbackLoaded_ = false;
        ctx_->callbackUiVec.clear();
    }
    
    if (ImGui::RadioButton(u8"对象回调", ctx_->selectedCallbackType_ == TypeObject)) {
        ctx_->selectedCallbackType_ = TypeObject;
        ctx_->callbackLoaded_ = false;
        ctx_->callbackUiVec.clear();
    }
    
    if (ImGui::RadioButton(u8"蓝屏回调", ctx_->selectedCallbackType_ == TypeBugCheck)) {
        ctx_->selectedCallbackType_ = TypeBugCheck;
        ctx_->callbackLoaded_ = false;
        ctx_->callbackUiVec.clear();
    }
    
    if (ImGui::RadioButton(u8"关机回调", ctx_->selectedCallbackType_ == TypeShutdown)) {
        ctx_->selectedCallbackType_ = TypeShutdown;
        ctx_->callbackLoaded_ = false;
        ctx_->callbackUiVec.clear();
    }
    
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // 右侧回调信息显示区域
    ImGui::BeginGroup();
    
    if (ImGui::Button(u8"刷新回调列表")) {
        CALLBACK_TYPE selectedType = (CALLBACK_TYPE)ctx_->selectedCallbackType_;
        ctx_->callbackUiVec = ctx_->arkR3.CallbackGetVec(selectedType);
        ctx_->callbackLoaded_ = true;
        ctx_->arkR3.Log("刷新回调列表: 类型=%d, 数量=%zu", selectedType, ctx_->callbackUiVec.size());
    }
    
    ImGui::Separator();
    
    // 回调信息表格
    if (ImGui::BeginTable("callback_table", 3, 
        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | 
        ImGuiTableFlags_SizingStretchProp | 
        ImGuiTableFlags_ScrollY, ImVec2(0, 400))) {
        
        ImGui::TableSetupColumn(u8"索引", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableSetupColumn(u8"回调地址", ImGuiTableColumnFlags_WidthFixed, 120);
        ImGui::TableSetupColumn(u8"模块路径");
        ImGui::TableHeadersRow();

        int row = 0;
        for (const auto& callback : ctx_->callbackUiVec) {
            
            ImGui::TableNextRow();
            
            // 索引列
            ImGui::TableSetColumnIndex(0);
            bool is_selected = (ctx_->selectedCallbackIndex_ == row);
            char selectableId[64];
            sprintf_s(selectableId, "%d##%d", callback.Index, row);
            if (ImGui::Selectable(selectableId, is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                ctx_->selectedCallbackIndex_ = row;
            }
            
            // 右键菜单
            char popupId[64];
            sprintf_s(popupId, "CallbackMenu##%d", row);
            if (ImGui::BeginPopupContextItem(popupId)) {
                if (ImGui::MenuItem(u8"删除回调")) {
                    // 删除
                    BOOL result = ctx_->arkR3.CallbackDelete(callback.Type, callback.Index);
                    if (result) {
                        ctx_->arkR3.Log("回调删除成功: 类型=%d, 索引=%d", callback.Type, callback.Index);
                        // 刷新回调列表
                        CALLBACK_TYPE selectedType = (CALLBACK_TYPE)ctx_->selectedCallbackType_;
                        ctx_->callbackUiVec = ctx_->arkR3.CallbackGetVec(selectedType);
                    } else {
                        ctx_->arkR3.Log("回调删除失败: 类型=%d, 索引=%d", callback.Type, callback.Index);
                    }
                }
                ImGui::EndPopup();
            }
            
            // 回调地址列
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%p", callback.CallbackEntry);
            
            // 模块路径列
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", callback.ModulePath);
            
            row++;
        }
        
        ImGui::EndTable();
    }
    
    // 统计信息
    ImGui::Separator();
    ImGui::Text(u8"总计: %zu 个回调  |  当前类型: %s", 
                ctx_->callbackUiVec.size(), 
                GetCallbackTypeName((CALLBACK_TYPE)ctx_->selectedCallbackType_));
    
    ImGui::EndGroup();
}

const char* CallbackWnd::GetCallbackTypeName(CALLBACK_TYPE type) {
    switch (type) {
        case TypeCreateProcess:     return u8"CreateProcess";
        case TypeCreateProcessEx:   return u8"CreateProcessEx";
        case TypeCreateThread:      return u8"CreateThread";
        case TypeLoadImage:         return u8"LoadImage";
        case TypeRegistry:          return u8"Registry";
        case TypeObject:            return u8"Object";
        case TypeBugCheck:          return u8"BugCheck";
        case TypeShutdown:          return u8"Shutdown";
        default:                    return u8"未知类型";
    }
}

const char* CallbackWnd::GetCallbackTypeDesc(CALLBACK_TYPE type) {
    switch (type) {
        case TypeCreateProcess:     return u8"监控进程创建和终止事件";
        case TypeCreateProcessEx:   return u8"监控进程创建和终止事件(扩展信息)";
        case TypeCreateThread:      return u8"监控线程创建和终止事件";
        case TypeLoadImage:         return u8"监控DLL/EXE模块加载事件";
        case TypeRegistry:          return u8"监控注册表操作事件";
        case TypeObject:            return u8"监控对象访问事件";
        case TypeBugCheck:          return u8"系统崩溃时触发";
        case TypeShutdown:          return u8"系统关机时触发";
        default:                    return u8"未知回调类型";
    }
}
