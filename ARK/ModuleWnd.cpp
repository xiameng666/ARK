#include "ModuleWnd.h"

ModuleWnd::ModuleWnd(Context* ctx)
    : ImguiWnd(ctx) 
{
}

void ModuleWnd::Render(bool* p_open)
{
    ImGui::Begin(u8"�ں�ģ��", p_open);
    
    RenderKernelModule();
    
    ImGui::End();

    ImGui::Begin(u8"��������", p_open);

    RenderDriverObject();

    ImGui::End();
}

//��Ⱦ��������
void ModuleWnd::RenderDriverObject() {
    if (ImGui::Button(u8"ˢ��")) {
        ctx_->driverObjectUiVec = ctx_->arkR3.DriverObjectGetVec();
    }

    ImGui::SameLine();
    if (ImGui::Button(u8"�Ƚ�����ģ��")) {
        ctx_->driverObjectUiVec = ctx_->arkR3.DriverHideDetect();
    }

    ImGui::SameLine();
    ImGui::Text(u8"������������: %d", (int)ctx_->driverObjectUiVec.size());

    ImGui::Separator();

    if (ctx_->driverObjectUiVec.empty()) {
        ImGui::Text(u8"��������");
        return;
    }

    // �����ʾ����������Ϣ
    if (ImGui::BeginTable(u8"���������б�", 4,
        ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {

        // ��ͷ
        ImGui::TableSetupColumn(u8"��������", ImGuiTableColumnFlags_WidthFixed, 200.0f);
        ImGui::TableSetupColumn(u8"��������", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn(u8"��ַ", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn(u8"��С", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableHeadersRow();

        // ������
        for (size_t i = 0; i < ctx_->driverObjectUiVec.size(); i++) {
            const auto& drvObj = ctx_->driverObjectUiVec[i];

            ImGui::TableNextRow();

            // ��������
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%ws", drvObj.DriverName);

            // ���������ַ
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%p", drvObj.DriverObject);

            // ��ַ
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%p", drvObj.DriverStart);

            // ��С
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("0x%X", drvObj.DriverSize);
        }

        ImGui::EndTable();
    }
}


// ��Ⱦȫ��ϵͳģ��
void ModuleWnd::RenderKernelModule()
{
    if (ImGui::Button(u8"ˢ��")) {
        DWORD moduleCount = ctx_->arkR3.ModuleGetCount();
        if (moduleCount > 0) {
            ctx_->globalModuleUiVec = ctx_->arkR3.ModuleGetVec(moduleCount);
        } else {
            ctx_->globalModuleUiVec.clear();
        }
    }

    ImGui::SameLine();
    ImGui::Checkbox(u8"ֻ��ʾ.sys����", &ctx_->showOnlySysFiles_);

    ImGui::SameLine();
    ImGui::Text(u8"ģ������: %d", (int)ctx_->globalModuleUiVec.size());
    
    ImGui::Separator();
    
    RenderModuleTable(ctx_->globalModuleUiVec, u8"�ں�ģ���б�");
}

// ��ProcessWnd�и���
void ModuleWnd::RenderModuleTable(const std::vector<MODULE_INFO>& moduleList, const char* windowTitle)
{
    if (moduleList.empty()) {
        ImGui::Text(u8"��������");
        return;
    }
    
    // ���������ʾģ����Ϣ
    if (ImGui::BeginTable(windowTitle, 5, 
        ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {
        
        // ��ͷ
        ImGui::TableSetupColumn(u8"ģ������", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn(u8"����ַ", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn(u8"��С", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn(u8"����˳��", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn(u8"����·��", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        
        // ������
        for (size_t i = 0; i < moduleList.size(); i++) {
            const auto& module = moduleList[i];

            if (ctx_->showOnlySysFiles_) {
                std::string moduleName = module.Name;
                std::transform(moduleName.begin(), moduleName.end(), moduleName.begin(), ::tolower);
                if (moduleName.find(".sys") == std::string::npos) {
                    continue;  // ������.sys�ļ�
                }
            }

            ImGui::TableNextRow();
            
            // ģ������
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", module.Name);
            
            // ����ַ
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%p", module.ImageBase);
            
            // ��С
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("0x%X", module.ImageSize);
            
            // ����˳��
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%d", module.LoadOrderIndex);
            
            // ����·��
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", module.FullPath);
        }
        
        ImGui::EndTable();
    }
} 
