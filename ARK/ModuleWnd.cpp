#include "ModuleWnd.h"

ModuleWnd::ModuleWnd(Context* ctx)
    : ImguiWnd(ctx) 
{
}

void ModuleWnd::Render(bool* p_open)
{
    ImGui::Begin(u8"内核模块", p_open);
    
    RenderKernelModule();
    
    ImGui::End();

    ImGui::Begin(u8"驱动对象", p_open);

    RenderDriverObject();

    ImGui::End();
}

//渲染驱动对象
void ModuleWnd::RenderDriverObject() {
    if (ImGui::Button(u8"刷新")) {
        ctx_->driverObjectUiVec = ctx_->arkR3.DriverObjectGetVec();
    }

    ImGui::SameLine();
    if (ImGui::Button(u8"比较隐藏模块")) {
        ctx_->driverObjectUiVec = ctx_->arkR3.DriverHideDetect();
    }

    ImGui::SameLine();
    ImGui::Text(u8"驱动对象数量: %d", (int)ctx_->driverObjectUiVec.size());

    ImGui::Separator();

    if (ctx_->driverObjectUiVec.empty()) {
        ImGui::Text(u8"暂无数据");
        return;
    }

    // 表格显示驱动对象信息
    if (ImGui::BeginTable(u8"驱动对象列表", 4,
        ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {

        // 表头
        ImGui::TableSetupColumn(u8"驱动名称", ImGuiTableColumnFlags_WidthFixed, 200.0f);
        ImGui::TableSetupColumn(u8"驱动对象", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn(u8"基址", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn(u8"大小", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableHeadersRow();

        // 数据行
        for (size_t i = 0; i < ctx_->driverObjectUiVec.size(); i++) {
            const auto& drvObj = ctx_->driverObjectUiVec[i];

            ImGui::TableNextRow();

            // 驱动名称
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%ws", drvObj.DriverName);

            // 驱动对象地址
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%p", drvObj.DriverObject);

            // 基址
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%p", drvObj.DriverStart);

            // 大小
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("0x%X", drvObj.DriverSize);
        }

        ImGui::EndTable();
    }
}


// 渲染全局系统模块
void ModuleWnd::RenderKernelModule()
{
    if (ImGui::Button(u8"刷新")) {
        DWORD moduleCount = ctx_->arkR3.ModuleGetCount();
        if (moduleCount > 0) {
            ctx_->globalModuleUiVec = ctx_->arkR3.ModuleGetVec(moduleCount);
        } else {
            ctx_->globalModuleUiVec.clear();
        }
    }

    ImGui::SameLine();
    ImGui::Checkbox(u8"只显示.sys驱动", &ctx_->showOnlySysFiles_);

    ImGui::SameLine();
    ImGui::Text(u8"模块数量: %d", (int)ctx_->globalModuleUiVec.size());
    
    ImGui::Separator();
    
    RenderModuleTable(ctx_->globalModuleUiVec, u8"内核模块列表");
}

// 在ProcessWnd中复用
void ModuleWnd::RenderModuleTable(const std::vector<MODULE_INFO>& moduleList, const char* windowTitle)
{
    if (moduleList.empty()) {
        ImGui::Text(u8"暂无数据");
        return;
    }
    
    // 创建表格显示模块信息
    if (ImGui::BeginTable(windowTitle, 5, 
        ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {
        
        // 表头
        ImGui::TableSetupColumn(u8"模块名称", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn(u8"基地址", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn(u8"大小", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn(u8"加载顺序", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn(u8"完整路径", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        
        // 数据行
        for (size_t i = 0; i < moduleList.size(); i++) {
            const auto& module = moduleList[i];

            if (ctx_->showOnlySysFiles_) {
                std::string moduleName = module.Name;
                std::transform(moduleName.begin(), moduleName.end(), moduleName.begin(), ::tolower);
                if (moduleName.find(".sys") == std::string::npos) {
                    continue;  // 跳过非.sys文件
                }
            }

            ImGui::TableNextRow();
            
            // 模块名称
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", module.Name);
            
            // 基地址
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%p", module.ImageBase);
            
            // 大小
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("0x%X", module.ImageSize);
            
            // 加载顺序
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%d", module.LoadOrderIndex);
            
            // 完整路径
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", module.FullPath);
        }
        
        ImGui::EndTable();
    }
} 
