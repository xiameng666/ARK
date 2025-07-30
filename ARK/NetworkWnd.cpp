#include "NetworkWnd.h"
#include <iostream>

NetworkWnd::NetworkWnd(Context* ctx) : ImguiWnd(ctx)
{

}

NetworkWnd::~NetworkWnd()
{

}

void NetworkWnd::Render(bool* p_open)
{
    if (!p_open || !*p_open)
    {
        return;
    }

    ImGui::Begin(u8"端口", p_open, ImGuiWindowFlags_AlwaysAutoResize);
    
    // 刷新按钮
    if (ImGui::Button(u8"刷新"))
    {
        ctx_->arkR3.NetworkPortVec_ = ctx_->arkR3.NetworkPortGetVec();
    }
    
    ImGui::SameLine();
    ImGui::Text("Total Ports: %zu", ctx_->arkR3.NetworkPortVec_.size());
    
    ImGui::Separator();
    
    if (ImGui::BeginTable("NetworkPorts", 7, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
    {
        ImGui::TableSetupColumn("Protocol", ImGuiTableColumnFlags_WidthStretch, 60.0f);
        ImGui::TableSetupColumn("Local Address", ImGuiTableColumnFlags_WidthStretch, 120.0f);
        ImGui::TableSetupColumn("Remote Address", ImGuiTableColumnFlags_WidthStretch, 120.0f);
        ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthStretch, 100.0f);
        ImGui::TableSetupColumn("Connection ID", ImGuiTableColumnFlags_WidthStretch, 100.0f);
        ImGui::TableSetupColumn("Process ID", ImGuiTableColumnFlags_WidthStretch, 80.0f);
        ImGui::TableSetupColumn("Process Path", ImGuiTableColumnFlags_WidthStretch, 200.0f);
        ImGui::TableHeadersRow();
        
        // 数据行
        for (const auto& port : ctx_->arkR3.NetworkPortVec_)
        {
            ImGui::TableNextRow();
            
            // Protocol
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(COLOR_CYAN, "%s", port.Protocol);
            
            // Local Address
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", port.LocalAddress);
            
            // Remote Address
            ImGui::TableSetColumnIndex(2);
            if (strlen(port.RemoteAddress) > 0)
            {
                ImGui::Text("%s", port.RemoteAddress);
            }
            else
            {
                ImGui::TextColored(COLOR_GRAY, "*:*");
            }
            
            // State
            ImGui::TableSetColumnIndex(3);
            if (strcmp(port.State, "LISTENING") == 0)
            {
                ImGui::TextColored(COLOR_GREEN, "%s", port.State);
            }
            else if (strcmp(port.State, "ESTABLISHED") == 0)
            {
                ImGui::TextColored(COLOR_YELLOW, "%s", port.State);
            }
            else if (strcmp(port.State, "TIME_WAIT") == 0 || strcmp(port.State, "CLOSE_WAIT") == 0)
            {
                ImGui::TextColored(COLOR_ORANGE, "%s", port.State);
            }
            else
            {
                ImGui::TextColored(COLOR_GRAY, "%s", port.State);
            }
            
            // Connection ID
            ImGui::TableSetColumnIndex(4);
            if (port.ConnectionId != 0)
            {
                ImGui::Text("0x%llX", port.ConnectionId);
            }
            else
            {
                ImGui::TextColored(COLOR_GRAY, "-");
            }
            
            // Process ID
            ImGui::TableSetColumnIndex(5);
            if (port.ProcessId != 0)
            {
                ImGui::Text("%u", port.ProcessId);
            }
            else
            {
                ImGui::TextColored(COLOR_GRAY, "-");
            }
            
            // Process Path
            ImGui::TableSetColumnIndex(6);
            if (strlen(port.ProcessPath) > 0)
            {
                ImGui::Text("%s", port.ProcessPath);
            }
            else
            {
                ImGui::TextColored(COLOR_GRAY, "-");
            }
        }
        
        ImGui::EndTable();
    }
    
    ImGui::End();
}
