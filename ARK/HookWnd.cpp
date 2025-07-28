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
    if (ImGui::Button(u8"ˢ��")) {
        ctx_->dispatchHookUiVec = ctx_->arkR3.DispatchHookGetVec();
    }
    
    ImGui::Separator();
    
    if (ImGui::BeginTable(u8"IO��ǲ����", 6, 
        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | 
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {
        
        ImGui::TableSetupColumn("Driver Name", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Code", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Current Address", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("�Ƿ�hook", ImGuiTableColumnFlags_WidthFixed, 80.0f);
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
    if (ImGui::Button(u8"ˢ��")) {
        ctx_->deviceStackUiVec = ctx_->arkR3.DeviceStackGetVec();
    }

    if (ImGui::BeginTable(u8"�豸ջ��Ϣ", 4,  // ��3�и�Ϊ4��
        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {

        ImGui::TableSetupColumn(u8"��������", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn(u8"����·��", ImGuiTableColumnFlags_WidthStretch, 250.0f);  // ����·����
        ImGui::TableSetupColumn(u8"�豸����", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn(u8"��������", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < ctx_->deviceStackUiVec.size(); i++) {
            const auto& stackInfo = ctx_->deviceStackUiVec[i];

            // ��ʾԭʼ������
            ImGui::TableNextRow();

            // ��������
            ImGui::TableSetColumnIndex(0);
            char ansiDriverName[128];
            WideCharToMultiByte(CP_ACP, 0, stackInfo.OriginalDriverName, -1,
                ansiDriverName, sizeof(ansiDriverName), NULL, NULL);

            if (stackInfo.FilterCount > 0) {
                ImGui::Text(u8"[ԭʼ] %s (%d�����)", ansiDriverName, stackInfo.FilterCount);
            }
            else {
                ImGui::Text(u8"[ԭʼ] %s", ansiDriverName);
            }

            // ����·��
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", stackInfo.OriginalDriverPath);

            // �豸����
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%p", stackInfo.OriginalDeviceObject);

            // ��������
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%p", stackInfo.OriginalDriverObject);

            // ��ʾ������������
            for (ULONG j = 0; j < stackInfo.FilterCount; j++) {
                const auto& filterInfo = stackInfo.Filters[j];

                ImGui::TableNextRow();

                // �������� (������ʾ)
                ImGui::TableSetColumnIndex(0);
                char ansiFilterName[128];
                memset(ansiFilterName, 0, sizeof(ansiFilterName));

                int convertResult = WideCharToMultiByte(CP_ACP, 0, filterInfo.DriverName,
                    wcslen(filterInfo.DriverName),
                    ansiFilterName, sizeof(ansiFilterName) - 1, NULL, NULL);

                ansiFilterName[convertResult] = '\0';
                ImGui::Text(u8"  ���� [����%d] %s", j + 1, ansiFilterName);

                // ��������·��
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", filterInfo.DriverPath);

                // �豸����
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%p", filterInfo.DeviceObject);

                // ��������
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%p", filterInfo.DriverObject);
            }
        }

        ImGui::EndTable();
    }

    if (ctx_->deviceStackUiVec.empty()) {
        ImGui::Text(u8"δ��⵽��Attach���豸ջ");
    }
}
