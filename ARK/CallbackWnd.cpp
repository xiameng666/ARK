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
void CallbackWnd::EnumCallback() {
    // 清空当前数据
    ctx_->callbackUiVec.clear();

    CALLBACK_TYPE allTypes[] = {
        TypeProcess,
        TypeThread,
        TypeImage,
        TypeRegistry,
        TypeObject,
        TypeBugCheck,
        TypeBugCheckReason,
        TypeShutdown
        /*            
                    */
    };

    // 遍历所有类型，获取回调并合并到一个vector中
    for (int i = 0; i < sizeof(allTypes) / sizeof(allTypes[0]); i++) {
        CALLBACK_TYPE currentType = allTypes[i];

        // 获取当前类型的回调
        auto typeCallbacks = ctx_->arkR3.CallbackGetVec(currentType);

        // 合并到总的vector中
        ctx_->callbackUiVec.insert(ctx_->callbackUiVec.end(),
            typeCallbacks.begin(),
            typeCallbacks.end());

       // ctx_->arkR3.Log("%s: %zu 个\n", GetCallbackTypeName(currentType), typeCallbacks.size());
    }

    ctx_->callbackLoaded_ = true;
}
void CallbackWnd::RenderCallbackWnd() {
   
    if (ImGui::Button(u8"刷新")) {
        EnumCallback();
    }
    
    //ImGui::SameLine();
    //
    //if (ImGui::Button(u8"清空列表")) {
    //    ctx_->callbackUiVec.clear();
    //    ctx_->callbackLoaded_ = false;
    //    ctx_->arkR3.Log("已清空回调列表");
    //}
    
    ImGui::Separator();
    
    if (ImGui::BeginTable("callback_table", 5, 
        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | 
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) { 
        
        ImGui::TableSetupColumn(u8"回调地址", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn(u8"类型", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn(u8"数组下标", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn(u8"模块路径", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(u8"Extra", ImGuiTableColumnFlags_WidthFixed, 120.0f); 
        ImGui::TableHeadersRow();

        int row = 0;
        for (const auto& callback : ctx_->callbackUiVec) {
            
            ImGui::TableNextRow();
            
            // 回调地址列（第0列）
            ImGui::TableSetColumnIndex(0);
            bool is_selected = (ctx_->selectedCallbackIndex_ == row);
            char addressText[32];
            sprintf_s(addressText, "%p", callback.CallbackEntry);
            char selectableId[64];
            sprintf_s(selectableId, "%s##%d_%d", addressText, callback.Type, callback.Index);
            if (ImGui::Selectable(selectableId, is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                ctx_->selectedCallbackIndex_ = row;
            }
            
            // 右键菜单 
            char popupId[64];
            sprintf_s(popupId, "CallbackMenu##%d_%p_%d", callback.Type, callback.CallbackEntry, callback.Index);
            if (ImGui::BeginPopupContextItem(popupId)) {
                if (ImGui::MenuItem(u8"删除回调")) {
                    // 删除回调 根据类型选择传递的数据
                    PVOID deleteData = nullptr;
                    
                    switch (callback.Type) {
                        case TypeProcess:
                        case TypeThread:
                        case TypeImage:
                            // 数组：传递函数地址，通过索引删除
                            deleteData = callback.CallbackEntry;
                            break;
                            
                        case TypeRegistry:
                        case TypeObject:
                        case TypeBugCheck:
                        case TypeBugCheckReason:
                        case TypeShutdown:
                            // 链表：传递Extra数据（Cookie/整个对象...）
                            deleteData = callback.Extra.CallbackExtra;
                            break;
                            
                        default:
                            deleteData = callback.CallbackEntry;
                            break;
                    }
                    
                    BOOL result = ctx_->arkR3.CallbackDelete(callback.Type, callback.Index, deleteData);
                    if (result) {
                        //ctx_->arkR3.Log("回调删除成功: 类型=%d, 索引=%d 地址=%p\n", callback.Type, callback.Index,callback.CallbackEntry);
                        
                        ImGui::CloseCurrentPopup();
                        EnumCallback();
                                              
                    } else {
                        ctx_->arkR3.LogErr("回调删除失败");
                    }
                }
                ImGui::EndPopup();
            }
            
            // 回调类型
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", GetCallbackTypeName(callback.Type));
            
            // 数组下标
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%d", callback.Index);
            
            // 模块路径
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", callback.ModulePath);

            // Extra
            ImGui::TableSetColumnIndex(4);
            //ImGui::Text("%p", callback.Extra.CallbackExtra);

            row++;
        }
        
        ImGui::EndTable();
    }
    
    // 统计信息
    ImGui::Separator();
    ImGui::Text(u8"总计: %zu 个系统回调\n", ctx_->callbackUiVec.size());
    
    // 按类型统计
    if (ctx_->callbackUiVec.size() > 0) {
        ImGui::SameLine();
        ImGui::Text(u8"  |  ");
        ImGui::SameLine();
        
        // 计算各类型数量
        int typeCount[10] = {0}; // 足够大的数组
        for (const auto& callback : ctx_->callbackUiVec) {
            if (callback.Type < 10) {
                typeCount[callback.Type]++;
            }
        }
        
        // 显示非零的类型统计
        bool first = true;
        CALLBACK_TYPE types[] = {TypeProcess, TypeThread, TypeImage, 
                                TypeRegistry, TypeObject, TypeBugCheck, TypeShutdown};
        
        for (int i = 0; i < sizeof(types)/sizeof(types[0]); i++) {
            if (typeCount[types[i]] > 0) {
                if (!first) {
                    ImGui::SameLine();
                    ImGui::Text(" | ");
                    ImGui::SameLine();
                }
                ImGui::Text("%s: %d", GetCallbackTypeName(types[i]), typeCount[types[i]]);
                first = false;
            }
        }
    }
}

const char* CallbackWnd::GetCallbackTypeName(CALLBACK_TYPE type) {
    switch (type) {
        case TypeProcess:           return u8"Process";     
        case TypeThread:            return u8"Thread";
        case TypeImage:             return u8"Image";
        case TypeRegistry:          return u8"Registry";
        case TypeObject:            return u8"Object";
        case TypeBugCheck:          return u8"BugCheck";
        case TypeBugCheckReason:    return u8"BugCheckReason";
        case TypeShutdown:          return u8"Shutdown";
        default:                    return u8"未知类型";
    }
}

/*
const char* CallbackWnd::GetCallbackTypeDesc(CALLBACK_TYPE type) {
    switch (type) {
        case TypeProcess:           return u8"监控进程创建和终止事件 (CreateProcess/Ex)";
        case TypeThread:            return u8"监控线程创建和终止事件";
        case TypeImage:             return u8"监控DLL/EXE模块加载事件";
        case TypeRegistry:          return u8"监控注册表操作事件";
        case TypeObject:            return u8"监控对象访问事件";
        case TypeBugCheck:          return u8"系统崩溃时触发";
        case TypeBugCheckReason:    return u8"系统崩溃原因回调";
        case TypeShutdown:          return u8"系统关机时触发";
        default:                    return u8"未知回调类型";
    }
}
*/
