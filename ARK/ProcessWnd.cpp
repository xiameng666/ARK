#include "ProcessWnd.h"
#include  "ModuleWnd.h"

ProcessWnd::ProcessWnd(Context* ctx) 
    :ImguiWnd(ctx)
{
    viewRenderers_[PROCESS] = [this]() { RenderProcessWnd(); };
    viewRenderers_[MEM] = [this]() { RenderProcessWnd(); };
}

void ProcessWnd::Render(bool* p_open)
{
    ImGui::Begin(u8"����", p_open);
    
    RenderProcessWnd();
    
    ImGui::End();
    
    if (ctx_->showProcessModuleWnd) {
        RenderProcessModuleWnd();
    }
}


void ProcessWnd::Flush(ProcSearchType type) {

    ctx_->processUiVec.clear();

    if (type == Links) {
        DWORD count = ctx_->arkR3.ProcessGetCount();
        ctx_->arkR3.Log("����%d\n", count);
        ctx_->processUiVec = ctx_->arkR3.ProcessGetVec(count);
    }
    else if (type == Mem){

    }
    
}

void ProcessWnd::RenderProcessWnd() {

    //�Զ�ˢ��
    SetAutoFlush(true);
    if (ActiveFlushWnd) {
        auto now = std::chrono::steady_clock::now();
        auto pased = std::chrono::duration_cast<std::chrono::seconds>(now - lastFlushTime);

        if (pased.count() >= flushSecond) {
            ctx_->arkR3.Log("�Զ�ˢ��");
            Flush(Links);
            lastFlushTime = now;
        }
    }

    //�ֶ�ˢ��
    if (ImGui::Button(u8"ˢ��")) {
        Flush(Links);
        lastFlushTime = std::chrono::steady_clock::now(); // ���ü�ʱ��
    }

    ImGui::Separator();

    if (ImGui::BeginTable("proc_table", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Sortable)) {
        ImGui::TableSetupColumn(u8"������", ImGuiTableColumnFlags_DefaultSort);           
        ImGui::TableSetupColumn(u8"����ID", ImGuiTableColumnFlags_DefaultSort);           
        ImGui::TableSetupColumn(u8"��PID", ImGuiTableColumnFlags_DefaultSort);            
        ImGui::TableSetupColumn(u8"ҳĿ¼��ַ", ImGuiTableColumnFlags_DefaultSort);       
        ImGui::TableSetupColumn(u8"EPROCESS��ַ", ImGuiTableColumnFlags_DefaultSort);     
        ImGui::TableHeadersRow();

        // ��������
        if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs()) {
            if (sorts_specs->SpecsDirty) {
                // �Խ���������������
                std::sort(ctx_->processUiVec.begin(), ctx_->processUiVec.end(), 
                    [sorts_specs](const PROCESS_INFO& a, const PROCESS_INFO& b) {
                        for (int n = 0; n < sorts_specs->SpecsCount; n++) {
                            const ImGuiTableColumnSortSpecs* sort_spec = &sorts_specs->Specs[n];
                            int delta = 0;
                            
                            switch (sort_spec->ColumnIndex) {
                                case 0: // ������
                                    delta = strcmp(a.ImageFileName, b.ImageFileName);
                                    break;
                                case 1: // ����ID
                                    delta = (a.ProcessId < b.ProcessId) ? -1 : (a.ProcessId > b.ProcessId) ? 1 : 0;
                                    break;
                                case 2: // ��PID
                                    delta = (a.ParentProcessId < b.ParentProcessId) ? -1 : (a.ParentProcessId > b.ParentProcessId) ? 1 : 0;
                                    break;
                                case 3: // ҳĿ¼��ַ
                                    delta = (a.DirectoryTableBase < b.DirectoryTableBase) ? -1 : (a.DirectoryTableBase > b.DirectoryTableBase) ? 1 : 0;
                                    break;
                                case 4: // EPROCESS��ַ
                                    delta = (a.EprocessAddr < b.EprocessAddr) ? -1 : (a.EprocessAddr > b.EprocessAddr) ? 1 : 0;
                                    break;
                            }
                            
                            if (delta != 0)
                                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? (delta < 0) : (delta > 0);
                        }
                        return false;
                    });
                sorts_specs->SpecsDirty = false;
            }
        }

        static int selected_index = -1;
        int row = 0;

        for (const auto& process : ctx_->processUiVec) { 
            ImGui::TableNextRow();

            
            // 0 ��ʾ������ + ����ѡ��
            ImGui::TableSetColumnIndex(0);
            bool is_selected = (selected_index == row);

            char selectableId[32];
            sprintf_s(selectableId, "##%u", process.ProcessId);  //pid��id

            if (ImGui::Selectable(selectableId, is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                selected_index = row;
            }

            // �Ҽ��˵�
            char popupId[64];
            sprintf_s(popupId, "ProcessMenu##%d", row);
            if (ImGui::BeginPopupContextItem(popupId)) {
                if (ImGui::MenuItem(u8"ǿ����ֹ����")) {

                    bool result = ctx_->arkR3.ProcessForceKill(process.ProcessId);
                    if (result) {
                        ctx_->arkR3.Log("�ɹ���ֹ���� %s (PID: %u)\n", process.ImageFileName, process.ProcessId);

                    }
                    else {
                        ctx_->arkR3.Log("��ֹ����ʧ�� %s (PID: %u)\n", process.ImageFileName, process.ProcessId);
                    }

                    Flush(Links);
                    lastFlushTime = std::chrono::steady_clock::now(); // ���ü�ʱ��

                }

                if (ImGui::MenuItem(u8"TODO1")) {
                    ctx_->targetPid_ = process.ProcessId;
                    sprintf_s(ctx_->processIdText_, "%u", process.ProcessId);
                    ctx_->showMemoryWindow = true;
                }
                if (ImGui::MenuItem(u8"TODO2")) {
                    ctx_->moduleTargetPid = process.ProcessId;
                    sprintf_s(ctx_->moduleTargetProcessName, "%s", process.ImageFileName);
                    ctx_->showProcessModuleWnd = true;

                    // ���ؽ���ģ��
                    //DWORD moduleCount = ctx_->arkR3.ProcessModuleGetCount(ctx_->moduleTargetPid);
                    //ctx_->processModuleUiVec = ctx_->arkR3.ProcessModuleGetVec(ctx_->moduleTargetPid, moduleCount);

                }
                ImGui::EndPopup();
            }

            // �ڵ�0����ʾ��������������Selectable���棩
            ImGui::SameLine(0, 0); 
            ImGui::Text("%s", process.ImageFileName);

            // 1 ����ID
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%u", process.ProcessId);

            // 2 ��PID
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%u", process.ParentProcessId);

            // 3 ҳĿ¼��ַ
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("0x%08X", process.DirectoryTableBase);

            // 4 EPROCESS��ַ
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%p", process.EprocessAddr);

            row++;
        }
        ImGui::EndTable();
    }

    ImGui::Text(u8"��������: %d", (int)ctx_->processUiVec.size());

    // ��ʾ�ڴ��������
    if (ctx_->showMemoryWindow) {
        RenderMemWnd(ctx_->targetPid_);
    }
}

void ProcessWnd::RenderMemWnd(DWORD pid)
{
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    ImGui::Begin(u8"�ڴ��д", &ctx_->showMemoryWindow);
    
    ImGui::Text(u8"Ŀ�����ID: %u", pid);
    ImGui::Separator();

    ImGui::Text(u8"��ַ:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    ImGui::InputText("##Address", ctx_->addressText_, sizeof(ctx_->addressText_));
    ImGui::SameLine();
    ImGui::Text(u8"��С:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::InputText("##Size", ctx_->sizeText_, sizeof(ctx_->sizeText_));

    if (ImGui::Button(u8"��")) {
        ULONG processId = strtoul(ctx_->processIdText_, NULL, 10);
        ULONG address = strtoul(ctx_->addressText_, NULL, 16);
        DWORD size = strtoul(ctx_->sizeText_, NULL, 10);

        //ctx_->arkR3.MemAttachRead(processId, address, size);
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"д")) {
        DWORD processId = strtoul(ctx_->processIdText_, NULL, 10);
        ULONG address = strtoul(ctx_->addressText_, NULL, 16);
        
        PVOID pData = ctx_->arkR3.GetBufferData();
        DWORD dataSize = ctx_->arkR3.GetDataSize();
        
        if (processId != 0 && address != 0 && pData && dataSize > 0) {
            //ctx_->arkR3.MemAttachWrite(processId, address, dataSize);
        } else {
            ctx_->arkR3.Log("processId != 0 && address != 0 && pData && dataSize > 0");
        }
    }

    ImGui::Separator();
    
    // ��ʾ�ڴ�༭��
    PVOID pData = ctx_->arkR3.GetBufferData();
    DWORD dataSize = ctx_->arkR3.GetDataSize();
    
    if (pData && dataSize > 0) {
        ImGui::Text(u8"�ڴ�༭��:");
        ULONG address = strtoul(ctx_->addressText_, NULL, 16);
        mem_edit.DrawContents(pData, dataSize, address);
    } else {
        ImGui::Text(u8"���ȶ�ȡ�ڴ�����");
    }

    ImGui::End();
}

// ��Ⱦ����ģ�鴰��
void ProcessWnd::RenderProcessModuleWnd()
{
    char windowTitle[128];
    sprintf_s(windowTitle, u8"����ģ�� - [%d] %s###ProcessModule", ctx_->moduleTargetPid, ctx_->moduleTargetProcessName);
    
    if (ImGui::Begin(windowTitle, &ctx_->showProcessModuleWnd)) {
        // ˢ�°�ť
        if (ImGui::Button(u8"ˢ��ģ��")) {
            //DWORD moduleCount = ctx_->arkR3.ProcessModuleGetCount(ctx_->moduleTargetPid);
            //ctx_->processModuleUiVec = ctx_->arkR3.ProcessModuleGetVec(ctx_->moduleTargetPid, moduleCount);
        }
        
        ImGui::SameLine();
        ImGui::Text(u8"ģ������: %d", (int)ctx_->processModuleUiVec.size());
        
        ImGui::Separator();
        
        // ֱ�ӵ��þ�̬����
        char tableTitle[64];
        sprintf_s(tableTitle, u8"����%dģ���б�", ctx_->moduleTargetPid);
        ModuleWnd::RenderModuleTable(ctx_->processModuleUiVec, tableTitle);
    }
    
    ImGui::End();
}
