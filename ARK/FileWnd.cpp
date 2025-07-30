#include "FileWnd.h"

FileWnd::FileWnd(Context* ctx) : ImguiWnd(ctx) {
}

void FileWnd::Render(bool* p_open) {
    ImGui::Begin("文件管理器", p_open);
    
    ImGui::Text("文件管理器窗口");
    ImGui::Text("这里可以添加文件管理功能");
    
    ImGui::End();
} 