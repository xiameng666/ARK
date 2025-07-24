#include "LogWnd.h"
/*
void LogWnd::AddLog(const char* log) {
    logs_.emplace_back(AnsiToUtf8(log));
    //logs_.push_back(log);
    scrollToBottom_ = true;
}

void LogWnd::Clear() {
    logs_.clear();
}

void LogWnd::Render(bool* p_open) {
    ImGui::Begin("Log", p_open);
    if (ImGui::Button(u8"Çå¿Õ")) Clear();
    ImGui::SameLine();
    //...

    ImGui::Separator();
    ImGui::BeginChild("LogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& line : logs_) {
        ImGui::TextUnformatted(line.c_str());
    }
    if (scrollToBottom_) {
        ImGui::SetScrollHereY(1.0f);
        scrollToBottom_ = false;
    }
    ImGui::EndChild();
    ImGui::End();
}
*/
void LogWnd::OnLog(const char* msg)
{
    AddLog(msg);
}

void LogWnd::AddLog(const char* log) {
    std::string u8log = exampleLog_.AnsiToUtf8(log);
    exampleLog_.AddLog("%s", u8log.c_str());
}

void LogWnd::Clear() { exampleLog_.Clear(); }

void LogWnd::Render(bool* p_open) {
    exampleLog_.Draw("Log", p_open);
}
