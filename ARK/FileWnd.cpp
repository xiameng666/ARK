#include "FileWnd.h"

FileWnd::FileWnd(Context* ctx) : ImguiWnd(ctx) {
}

void FileWnd::Render(bool* p_open) {
    ImGui::Begin("�ļ�������", p_open);
    
    ImGui::Text("�ļ�����������");
    ImGui::Text("�����������ļ�������");
    
    ImGui::End();
} 