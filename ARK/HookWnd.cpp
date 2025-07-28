#include "HookWnd.h"
#include <algorithm>

HookWnd::HookWnd(Context* ctx)
    : ImguiWnd(ctx)
{

}

void HookWnd::Render(bool* p_open)
{
    ImGui::Begin("Hook", p_open);
    
    //RenderMJHookWnd();
    RenderDeviceStackWnd();
    ImGui::End();
}

void HookWnd::RenderMJHookWnd()
{
    if (ImGui::Button(u8"刷新")) {
        ctx_->dispatchHookUiVec = ctx_->arkR3.DispatchHookGetVec();
    }
    
    ImGui::Separator();
    
    if (ImGui::BeginTable(u8"IO派遣函数", 6, 
        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | 
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {
        
        ImGui::TableSetupColumn("Driver Name", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Code", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Current Address", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("是否被hook", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("hook module", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        
        int row = 0;
        for (const auto& hookInfo : ctx_->dispatchHookUiVec) {
            ImGui::TableNextRow();
            
            // Driver Name
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", hookInfo.DriverName);
            
            // Function Name  
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", hookInfo.FunctionName);
            
            // Function Code
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%d", hookInfo.MajorFunctionCode);
            
            // Current Address
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%p", hookInfo.CurrentAddress);
            
            // Hook Status
            ImGui::TableSetColumnIndex(4);
            if (hookInfo.IsHooked) {
                ImGui::TextColored(COLOR_RED, "HOOKED");
            } else {
                ImGui::TextColored(COLOR_GREEN, "Normal");
            }
            
            // Hook Module
            ImGui::TableSetColumnIndex(5);
            if (hookInfo.IsHooked) {
                ImGui::Text("%s", hookInfo.CurrentModule);
            } else {
                ImGui::TextColored(COLOR_GRAY, "N/A");
            }
            
            row++;
        }
        
        ImGui::EndTable();
    }
}

void HookWnd::RenderDeviceStackWnd()
{
    if (ImGui::Button(u8"刷新")) {
        ctx_->deviceStackUiVec = ctx_->arkR3.DeviceStackGetVec();
    }

    if (ImGui::BeginTable(u8"设备栈信息", 4,  // 从3列改为4列
        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {

        ImGui::TableSetupColumn(u8"驱动名称", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn(u8"驱动路径", ImGuiTableColumnFlags_WidthStretch, 250.0f);  // 新增路径列
        ImGui::TableSetupColumn(u8"设备对象", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn(u8"驱动对象", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < ctx_->deviceStackUiVec.size(); i++) {
            const auto& stackInfo = ctx_->deviceStackUiVec[i];

            // 显示原始驱动行
            ImGui::TableNextRow();

            // 驱动名称
            ImGui::TableSetColumnIndex(0);
            char ansiDriverName[128];
            WideCharToMultiByte(CP_ACP, 0, stackInfo.OriginalDriverName, -1,
                ansiDriverName, sizeof(ansiDriverName), NULL, NULL);

            if (stackInfo.FilterCount > 0) {
                ImGui::Text(u8"[原始] %s (%d层过滤)", ansiDriverName, stackInfo.FilterCount);
            }
            else {
                ImGui::Text(u8"[原始] %s", ansiDriverName);
            }

            // 驱动路径
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", stackInfo.OriginalDriverPath);

            // 设备对象
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%p", stackInfo.OriginalDeviceObject);

            // 驱动对象
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%p", stackInfo.OriginalDriverObject);

            // 显示过滤驱动子行
            for (ULONG j = 0; j < stackInfo.FilterCount; j++) {
                const auto& filterInfo = stackInfo.Filters[j];

                ImGui::TableNextRow();

                // 驱动名称 (缩进显示)
                ImGui::TableSetColumnIndex(0);
                char ansiFilterName[128];
                memset(ansiFilterName, 0, sizeof(ansiFilterName));

                int convertResult = WideCharToMultiByte(CP_ACP, 0, filterInfo.DriverName,
                    wcslen(filterInfo.DriverName),
                    ansiFilterName, sizeof(ansiFilterName) - 1, NULL, NULL);

                ansiFilterName[convertResult] = '\0';
                ImGui::Text(u8"  └─ [过滤%d] %s", j + 1, ansiFilterName);

                // 过滤驱动路径
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", filterInfo.DriverPath);

                // 设备对象
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%p", filterInfo.DeviceObject);

                // 驱动对象
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%p", filterInfo.DriverObject);
            }
        }

        ImGui::EndTable();
    }

    if (ctx_->deviceStackUiVec.empty()) {
        ImGui::Text(u8"未检测到被Attach的设备栈");
    }
}
