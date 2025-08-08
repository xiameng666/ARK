#include "KernelWnd.h"


KernelWnd::KernelWnd(Context* ctx)
    : ImguiWnd(ctx)
{
    viewRenderers_[GDT] = [this]() { RenderGDTTable(); };
    viewRenderers_[IDT] = [this]() { RenderIDTTable(); };
    viewRenderers_[SSDT] = [this]() { RenderSSDTTable(); };
    viewRenderers_[SHADOWSSDT] = [this]() { RenderShadowSSDTTable(); };
}

void KernelWnd::Render(bool* p_open)
{
    ImGui::Begin(u8"内核", p_open);

    ImVec2 available = ImGui::GetContentRegionAvail();

    if (ImGui::BeginTable("KernelLayout", 2,
        ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable,
        available)){
        ImGui::TableSetupColumn(u8"导航", ImGuiTableColumnFlags_WidthFixed,200.0f);
        ImGui::TableSetupColumn(u8"内容", ImGuiTableColumnFlags_WidthStretch,800.0f);
        ImGui::TableNextRow();

        RenderLeftBar();
        ImGui::TableSetColumnIndex(1);
        auto it = viewRenderers_.find(ctx_->currentView);
        if (it != viewRenderers_.end()) {
            it->second();
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

const char* KernelWnd::GetInterruptPurpose(ULONG index) {
    switch (index) {
    case 0x00: return "Divide Error";
    case 0x01: return "Debug Exception";
    case 0x02: return "NMI";
    case 0x03: return "Breakpoint";
    case 0x04: return "Overflow";
    case 0x0D: return "General Protection";
    case 0x0E: return "Page Fault";
    case 0x2E: return "System Call (INT 2Eh)";
    default:
        if (index >= 0x30 && index <= 0xFF) {
            return "Hardware/Software Interrupt";
        }
        return "Exception/Reserved";
    }
}

const char* KernelWnd::GetIDTDescriptorType(USHORT type) {
    switch (type & 0xF) {
    case 0x5:
        return "Task Gate (32bit)";

    case 0xC:
        return "Call Gate";

    case 0xE:
        return "Interrupt Gate";

    case 0xF:
        return "Trap Gate";

    default:
        return "Invalid";
    }
}

void KernelWnd::RenderIDTTable()
{
    // 刷新按钮
    if (ImGui::Button(u8"刷新")) {
        ctx_->idtUiVec = ctx_->arkR3.IdtGetVec();
    }
    ImGui::SameLine();
    if (ImGui::Button(u8"恢复")) {
        ctx_->arkR3.RestoreIDT();
    }

    if (ImGui::BeginTable("IDT", 7,
        ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody |
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Sortable)) {

        // 表头
        ImGui::TableSetupColumn(u8"CPU", ImGuiTableColumnFlags_WidthStretch, 50);
        ImGui::TableSetupColumn(u8"ID", ImGuiTableColumnFlags_WidthStretch, 60);
        ImGui::TableSetupColumn(u8"函数名", ImGuiTableColumnFlags_WidthStretch, 200);
        ImGui::TableSetupColumn(u8"SEL", ImGuiTableColumnFlags_WidthStretch, 60);
        ImGui::TableSetupColumn(u8"DPL", ImGuiTableColumnFlags_WidthStretch, 60);
        ImGui::TableSetupColumn(u8"地址", ImGuiTableColumnFlags_WidthStretch, 120);
        ImGui::TableSetupColumn(u8"模块路径", ImGuiTableColumnFlags_WidthStretch);
        //ImGui::TableSetupColumn(u8"类型", ImGuiTableColumnFlags_WidthStretch, 120);

        ImGui::TableHeadersRow();

        for (const auto& idt : ctx_->idtUiVec) {
            ImGui::TableNextRow();

            // CPU索引
            ImGui::TableNextColumn();
            ImGui::Text("%d", idt.CpuId);

            // 中断索引(ID)
            ImGui::TableNextColumn();
            ImGui::Text("0x%02X", idt.id);

            // 函数名
            ImGui::TableNextColumn();
            if (strlen(idt.funcName) > 0) {
                ImGui::Text("%s", idt.funcName);
            }
            else {
                ImGui::TextColored(COLOR_LIGHT_GRAY, u8"未知");
            }

            // 段选择子(SEL)
            ImGui::TableNextColumn();
            ImGui::Text("0x%04X", idt.Selector);

            // 特权级(DPL)
            ImGui::TableNextColumn();
            ImVec4 dplColor;
            switch (idt.Dpl) {
            case 0: dplColor = COLOR_RED; break;    // 内核级
            case 3: dplColor = COLOR_GREEN; break;  // 用户级
            }
            ImGui::TextColored(dplColor, "%d", idt.Dpl);

            // 处理程序地址
            ImGui::TableNextColumn();
            ImGui::Text("0x%p", (void*)idt.Address);

            // 模块路径
            ImGui::TableNextColumn();
            if (strlen(idt.Path) > 0) {
                ImGui::Text("%s", idt.Path);
            }
            else {
                ImGui::TextColored(COLOR_LIGHT_GRAY, u8"N/A");
            }

            /*
            ImGui::TableNextColumn();
            const char* typeStr = GetIDTDescriptorType(idt.Type, idt.Dpl);
            ImGui::Text("%s", typeStr);
            */
        }

        ImGui::EndTable();

        // 显示统计信息
        ImGui::Text(u8"显示全部 %d 个IDT表项", (int)ctx_->idtUiVec.size());
    }
}

void KernelWnd::RenderSSDTTable()
{
    // 刷新按钮
    if (ImGui::Button(u8"刷新")) {
        ctx_->ssdtUiVec_ = ctx_->arkR3.SSDTGetVec();
    }

    ImGui::SameLine();
    if (ImGui::Button(u8"恢复全部")) {
        ctx_->arkR3.RestoreSSdt();
    }

    if (ImGui::BeginTable("SSDT", 3, 
        ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody |
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {
    
        // 列表头
        ImGui::TableSetupColumn(u8"索引", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableSetupColumn(u8"函数地址", ImGuiTableColumnFlags_WidthFixed, 120);
        ImGui::TableSetupColumn(u8"函数名", ImGuiTableColumnFlags_WidthStretch);


        ImGui::TableHeadersRow();
        
        for (const auto& ssdt : ctx_->ssdtUiVec_) {
            ImGui::TableNextRow();
            
            // 索引
            ImGui::TableNextColumn();
            ImGui::Text("%d", ssdt.Index);

            // 函数地址
            ImGui::TableNextColumn();
            ImGui::Text("0x%p", ssdt.FunctionAddress);

            // 函数名
            ImGui::TableNextColumn();
            ImGui::Text("%s", ssdt.FunctionName);
    

        }
    
        ImGui::EndTable();
    }
}

void KernelWnd::RenderGDTTable()
{
    static bool filterInvalidSegments = false;  // 默认过滤无效段
  
    if (ImGui::Button(u8"刷新")) {
        ctx_->gdtUiVec = ctx_->arkR3.GDTGetVec();
    }
    
    ImGui::SameLine();
    ImGui::Checkbox(u8"过滤无效段", &filterInvalidSegments);
    
    if (filterInvalidSegments) {
        ImGui::SameLine();
        ImGui::TextColored(COLOR_INFO, u8"(隐藏 Type=0、Limit=0 且 NP 的段)");
    }

    if (ImGui::BeginTable(u8"GDT", 9, 
        ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody |
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Sortable)) {

        ImGui::TableSetupColumn(u8"CPU");
        ImGui::TableSetupColumn(u8"Sel");
        ImGui::TableSetupColumn(u8"Base");
        ImGui::TableSetupColumn(u8"Limit");
        //ImGui::TableSetupColumn(u8"位数");     // 64/32
        ImGui::TableSetupColumn(u8"粒度");     // PAGE/BYTE
        ImGui::TableSetupColumn(u8"特权级");   // 0/3
        ImGui::TableSetupColumn(u8"Present");  // P/NP
        ImGui::TableSetupColumn(u8"Type");
        ImGui::TableSetupColumn(u8"段描述");

        ImGui::TableHeadersRow();
    
        int validCount = 0;
        int totalCount = 0;
        
        for (const auto& gdt : ctx_->gdtUiVec) {
            totalCount++;
            
            if (filterInvalidSegments) {
                // 过滤
                if (gdt.type == 0 || gdt.limit == 0 && !gdt.p) {
                    continue;
                }
            }
            
            validCount++;
            ImGui::TableNextRow();
    
            // CPU序号
            ImGui::TableNextColumn();
            ImGui::Text("%d", gdt.cpuIndex);
    
            // 段选择子
            ImGui::TableNextColumn();
            ImGui::Text("0x%04X", gdt.selector);
    
            // 基址
            ImGui::TableNextColumn();
            if (gdt.is_system_64 || gdt.base > 0xFFFFFFFF) {
                ImGui::Text("0x%016llX", gdt.base); 
            } else {
                ImGui::Text("0x%08X", (DWORD)gdt.base);  
            }

            // 界限
            ImGui::TableNextColumn();

            if (gdt.limit == 0) {
                ImGui::TextColored(COLOR_LIGHT_GRAY, "0x%04X", gdt.limit);
            } else {
                ImGui::Text("0x%04X", gdt.limit);
            }

            // 位数
            // ImGui::TableNextColumn();
            //ImGui::TextColored(gdt.l ? COLOR_CYAN : COLOR_GRAY, gdt.l ? "64" : "32");

            // 段粒度
            ImGui::TableNextColumn();
            ImGui::Text("%s", gdt.g ? u8"PAGE" : u8"BYTE");
    
            // 段特权级
            ImGui::TableNextColumn();
            ImVec4 dplColor;
            switch (gdt.dpl) {
            case 0: dplColor = COLOR_RED; break;    
            case 3: dplColor = COLOR_GREEN; break;  
            default: dplColor = COLOR_YELLOW; break;
            }
            ImGui::TextColored(dplColor, "%d", gdt.dpl);

            // Present
            ImGui::TableNextColumn();
            if (gdt.p) {
                ImGui::TextColored(COLOR_GREEN, "P");
            } else {
                ImGui::TextColored(COLOR_GRAY, "NP");
            }

            // Type 
            ImGui::TableNextColumn();
            ImGui::Text("0x%X", gdt.type);

            // 段描述
            ImGui::TableNextColumn();
            ImGui::Text("%s", gdt.typeDesc);

        }
    
        ImGui::EndTable();
        
        // 显示统计信息
        if (filterInvalidSegments) {
            ImGui::Text(u8"显示 %d/%d 个有效段描述符 (过滤了 %d 个无效段)", 
                       validCount, totalCount, totalCount - validCount);
        } else {
            ImGui::Text(u8"显示全部 %d 个段描述符", totalCount);
        }
    }
}

void KernelWnd::RenderShadowSSDTTable()
{
    // 刷新按钮
    if (ImGui::Button(u8"刷新")) {
        ctx_->shadowSsdtUiVec = ctx_->arkR3.ShadowSSDTGetVec();
    }
    ImGui::SameLine();
    if (ImGui::Button(u8"恢复全部")) {
        ctx_->arkR3.RestoreShadowSSdt();
    }

    if (ImGui::BeginTable("ShadowSSDT", 3,
        ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody |
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {

        // 表头
        ImGui::TableSetupColumn(u8"索引", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn(u8"函数地址", ImGuiTableColumnFlags_WidthFixed, 120);
        ImGui::TableSetupColumn(u8"函数名", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableHeadersRow();

        for (const auto& shadowSsdt : ctx_->shadowSsdtUiVec) {
            ImGui::TableNextRow();

            // 索引 (显示为0x1000+)
            ImGui::TableNextColumn();
            ImGui::Text("0x%04X", shadowSsdt.Index);

            // 函数地址
            ImGui::TableNextColumn();
            ImGui::Text("0x%p", shadowSsdt.FunctionAddress);

            // 函数名
            ImGui::TableNextColumn();
            if (strstr(shadowSsdt.FunctionName, "NtUser") != nullptr) {
                ImGui::TextColored(COLOR_CYAN, "%s", shadowSsdt.FunctionName);  // 用户界面函数
            }
            else if (strstr(shadowSsdt.FunctionName, "NtGdi") != nullptr) {
                ImGui::TextColored(COLOR_GREEN, "%s", shadowSsdt.FunctionName); // 图形设备函数
            }
            else {
                ImGui::Text("%s", shadowSsdt.FunctionName);
            }
        }

        ImGui::EndTable();

        // 显示统计信息
        ImGui::Text(u8"显示全部 %d 个ShadowSSDT表项", (int)ctx_->shadowSsdtUiVec.size());
    }
}

void KernelWnd::RenderLeftBar()
{
    ImGui::TableSetColumnIndex(0);
    ImGui::Text(u8" ");
    ImGui::Separator();
    
    NAV_SECTION(u8"内存管理",
        VIEW_ITEM(u8"GDT", GDT);
        VIEW_ITEM(u8"IDT", IDT);
        VIEW_ITEM(u8"SSDT", SSDT);
        VIEW_ITEM(u8"ShadowSSDT", SHADOWSSDT);
    );
}

