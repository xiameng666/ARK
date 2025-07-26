#include "CallbackWnd.h"
#include <algorithm>

CallbackWnd::CallbackWnd(Context* ctx) 
    : ImguiWnd(ctx)
{
    viewRenderers_[VIEWCALLBACK] = [this]() { RenderCallbackWnd(); };
}

void CallbackWnd::Render(bool* p_open)
{
    ImGui::Begin(u8"ϵͳ�ص�", p_open);
    
    RenderCallbackWnd();
    
    ImGui::End();
}
void CallbackWnd::EnumCallback() {
    // ��յ�ǰ����
    ctx_->callbackUiVec.clear();

    CALLBACK_TYPE allTypes[] = {
        TypeProcess,
        TypeThread,
        TypeImage,
        /*            
                    TypeRegistry,
                    TypeObject,
                    TypeBugCheck,
                    TypeShutdown */
    };

    // �����������ͣ���ȡ�ص����ϲ���һ��vector��
    for (int i = 0; i < sizeof(allTypes) / sizeof(allTypes[0]); i++) {
        CALLBACK_TYPE currentType = allTypes[i];

        // ��ȡ��ǰ���͵Ļص�
        auto typeCallbacks = ctx_->arkR3.CallbackGetVec(currentType);

        // �ϲ����ܵ�vector��
        ctx_->callbackUiVec.insert(ctx_->callbackUiVec.end(),
            typeCallbacks.begin(),
            typeCallbacks.end());

       // ctx_->arkR3.Log("%s: %zu ��\n", GetCallbackTypeName(currentType), typeCallbacks.size());
    }

    ctx_->callbackLoaded_ = true;
}
void CallbackWnd::RenderCallbackWnd() {
   
    if (ImGui::Button(u8"ˢ��")) {
        EnumCallback();
    }
    
    //ImGui::SameLine();
    //
    //if (ImGui::Button(u8"����б�")) {
    //    ctx_->callbackUiVec.clear();
    //    ctx_->callbackLoaded_ = false;
    //    ctx_->arkR3.Log("����ջص��б�");
    //}
    
    ImGui::Separator();
    
    if (ImGui::BeginTable("callback_table", 5, 
        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | 
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) { 
        
        ImGui::TableSetupColumn(u8"�ص���ַ", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn(u8"����", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn(u8"�����±�", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn(u8"ģ��·��", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(u8"Extra", ImGuiTableColumnFlags_WidthFixed, 120.0f); 
        ImGui::TableHeadersRow();

        int row = 0;
        for (const auto& callback : ctx_->callbackUiVec) {
            
            ImGui::TableNextRow();
            
            // �ص���ַ�У���0�У�
            ImGui::TableSetColumnIndex(0);
            bool is_selected = (ctx_->selectedCallbackIndex_ == row);
            char addressText[32];
            sprintf_s(addressText, "%p", callback.CallbackEntry);
            char selectableId[64];
            sprintf_s(selectableId, "%s##%d_%d", addressText, callback.Type, callback.Index);
            if (ImGui::Selectable(selectableId, is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                ctx_->selectedCallbackIndex_ = row;
            }
            
            // �Ҽ��˵�
            char popupId[64];
            sprintf_s(popupId, "CallbackMenu##%p", callback.CallbackEntry);
            if (ImGui::BeginPopupContextItem(popupId)) {
                if (ImGui::MenuItem(u8"ɾ���ص�")) {
                    // ɾ���ص�
                    BOOL result = ctx_->arkR3.CallbackDelete(callback.Type, callback.Index,callback.CallbackEntry);
                    if (result) {
                        //ctx_->arkR3.Log("�ص�ɾ���ɹ�: ����=%d, ����=%d ��ַ=%p\n", callback.Type, callback.Index,callback.CallbackEntry);
                        
                        ImGui::CloseCurrentPopup();
                        EnumCallback();
                                              
                    } else {
                        ctx_->arkR3.LogErr("�ص�ɾ��ʧ��");
                    }
                }
                ImGui::EndPopup();
            }
            
            // �ص�����
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", GetCallbackTypeName(callback.Type));
            
            // �����±�
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%d", callback.Index);
            
            // ģ��·��
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", callback.ModulePath);

            // Extra
            ImGui::TableSetColumnIndex(4);
            //ImGui::Text("%p", callback.Extra.CallbackExtra);

            row++;
        }
        
        ImGui::EndTable();
    }
    
    // ͳ����Ϣ
    ImGui::Separator();
    ImGui::Text(u8"�ܼ�: %zu ��ϵͳ�ص�\n", ctx_->callbackUiVec.size());
    
    // ������ͳ��
    if (ctx_->callbackUiVec.size() > 0) {
        ImGui::SameLine();
        ImGui::Text(u8"  |  ");
        ImGui::SameLine();
        
        // �������������
        int typeCount[10] = {0}; // �㹻�������
        for (const auto& callback : ctx_->callbackUiVec) {
            if (callback.Type < 10) {
                typeCount[callback.Type]++;
            }
        }
        
        // ��ʾ���������ͳ��
        bool first = true;
        CALLBACK_TYPE types[] = {TypeProcess, TypeThread, TypeImage, 
                                TypeRegistry, TypeObject, TypeBugCheck, TypeShutdown};
        
        for (int i = 0; i < sizeof(types)/sizeof(types[0]); i++) {
            if (typeCount[types[i]] > 0) {
                if (!first) {
                    ImGui::SameLine();
                    ImGui::Text(" | ");
                    ImGui::SameLine();
                }
                ImGui::Text("%s: %d", GetCallbackTypeName(types[i]), typeCount[types[i]]);
                first = false;
            }
        }
    }
}

const char* CallbackWnd::GetCallbackTypeName(CALLBACK_TYPE type) {
    switch (type) {
        case TypeProcess:           return u8"Process";     
        case TypeThread:            return u8"Thread";
        case TypeImage:             return u8"Image";
        case TypeRegistry:          return u8"Registry";
        case TypeObject:            return u8"Object";
        case TypeBugCheck:          return u8"BugCheck";
        case TypeShutdown:          return u8"Shutdown";
        default:                    return u8"δ֪����";
    }
}

const char* CallbackWnd::GetCallbackTypeDesc(CALLBACK_TYPE type) {
    switch (type) {
        case TypeProcess:           return u8"��ؽ��̴�������ֹ�¼� (CreateProcess/Ex)";
        case TypeThread:            return u8"����̴߳�������ֹ�¼�";
        case TypeImage:             return u8"���DLL/EXEģ������¼�";
        case TypeRegistry:          return u8"���ע�������¼�";
        case TypeObject:            return u8"��ض�������¼�";
        case TypeBugCheck:          return u8"ϵͳ����ʱ����";
        case TypeShutdown:          return u8"ϵͳ�ػ�ʱ����";
        default:                    return u8"δ֪�ص�����";
    }
}
