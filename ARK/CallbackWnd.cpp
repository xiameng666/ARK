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

void CallbackWnd::RenderCallbackWnd() {
    
    // �ص�����ѡ������ - ʹ��RadioButton
    ImGui::Text(u8"�ص�����:");
    ImGui::Separator();
    
    // ���ص�����ѡ������
    ImGui::BeginChild("callback_types", ImVec2(200, 0), true);
    
    if (ImGui::RadioButton(u8"���̴����ص�", ctx_->selectedCallbackType_ == TypeCreateProcess)) {
        ctx_->selectedCallbackType_ = TypeCreateProcess;
        ctx_->callbackLoaded_ = false;
        ctx_->callbackUiVec.clear();
    }
    
    if (ImGui::RadioButton(u8"���̴����ص�(��չ)", ctx_->selectedCallbackType_ == TypeCreateProcessEx)) {
        ctx_->selectedCallbackType_ = TypeCreateProcessEx;
        ctx_->callbackLoaded_ = false;
        ctx_->callbackUiVec.clear();
    }
    
    if (ImGui::RadioButton(u8"�̴߳����ص�", ctx_->selectedCallbackType_ == TypeCreateThread)) {
        ctx_->selectedCallbackType_ = TypeCreateThread;
        ctx_->callbackLoaded_ = false;
        ctx_->callbackUiVec.clear();
    }
    
    if (ImGui::RadioButton(u8"ӳ����ػص�", ctx_->selectedCallbackType_ == TypeLoadImage)) {
        ctx_->selectedCallbackType_ = TypeLoadImage;
        ctx_->callbackLoaded_ = false;
        ctx_->callbackUiVec.clear();
    }
    
    if (ImGui::RadioButton(u8"ע���ص�", ctx_->selectedCallbackType_ == TypeRegistry)) {
        ctx_->selectedCallbackType_ = TypeRegistry;
        ctx_->callbackLoaded_ = false;
        ctx_->callbackUiVec.clear();
    }
    
    if (ImGui::RadioButton(u8"����ص�", ctx_->selectedCallbackType_ == TypeObject)) {
        ctx_->selectedCallbackType_ = TypeObject;
        ctx_->callbackLoaded_ = false;
        ctx_->callbackUiVec.clear();
    }
    
    if (ImGui::RadioButton(u8"�����ص�", ctx_->selectedCallbackType_ == TypeBugCheck)) {
        ctx_->selectedCallbackType_ = TypeBugCheck;
        ctx_->callbackLoaded_ = false;
        ctx_->callbackUiVec.clear();
    }
    
    if (ImGui::RadioButton(u8"�ػ��ص�", ctx_->selectedCallbackType_ == TypeShutdown)) {
        ctx_->selectedCallbackType_ = TypeShutdown;
        ctx_->callbackLoaded_ = false;
        ctx_->callbackUiVec.clear();
    }
    
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // �Ҳ�ص���Ϣ��ʾ����
    ImGui::BeginGroup();
    
    if (ImGui::Button(u8"ˢ�»ص��б�")) {
        CALLBACK_TYPE selectedType = (CALLBACK_TYPE)ctx_->selectedCallbackType_;
        ctx_->callbackUiVec = ctx_->arkR3.CallbackGetVec(selectedType);
        ctx_->callbackLoaded_ = true;
        ctx_->arkR3.Log("ˢ�»ص��б�: ����=%d, ����=%zu", selectedType, ctx_->callbackUiVec.size());
    }
    
    ImGui::Separator();
    
    // �ص���Ϣ���
    if (ImGui::BeginTable("callback_table", 3, 
        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | 
        ImGuiTableFlags_SizingStretchProp | 
        ImGuiTableFlags_ScrollY, ImVec2(0, 400))) {
        
        ImGui::TableSetupColumn(u8"����", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableSetupColumn(u8"�ص���ַ", ImGuiTableColumnFlags_WidthFixed, 120);
        ImGui::TableSetupColumn(u8"ģ��·��");
        ImGui::TableHeadersRow();

        int row = 0;
        for (const auto& callback : ctx_->callbackUiVec) {
            
            ImGui::TableNextRow();
            
            // ������
            ImGui::TableSetColumnIndex(0);
            bool is_selected = (ctx_->selectedCallbackIndex_ == row);
            char selectableId[64];
            sprintf_s(selectableId, "%d##%d", callback.Index, row);
            if (ImGui::Selectable(selectableId, is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                ctx_->selectedCallbackIndex_ = row;
            }
            
            // �Ҽ��˵�
            char popupId[64];
            sprintf_s(popupId, "CallbackMenu##%d", row);
            if (ImGui::BeginPopupContextItem(popupId)) {
                if (ImGui::MenuItem(u8"ɾ���ص�")) {
                    // ɾ��
                    BOOL result = ctx_->arkR3.CallbackDelete(callback.Type, callback.Index);
                    if (result) {
                        ctx_->arkR3.Log("�ص�ɾ���ɹ�: ����=%d, ����=%d", callback.Type, callback.Index);
                        // ˢ�»ص��б�
                        CALLBACK_TYPE selectedType = (CALLBACK_TYPE)ctx_->selectedCallbackType_;
                        ctx_->callbackUiVec = ctx_->arkR3.CallbackGetVec(selectedType);
                    } else {
                        ctx_->arkR3.Log("�ص�ɾ��ʧ��: ����=%d, ����=%d", callback.Type, callback.Index);
                    }
                }
                ImGui::EndPopup();
            }
            
            // �ص���ַ��
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%p", callback.CallbackEntry);
            
            // ģ��·����
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", callback.ModulePath);
            
            row++;
        }
        
        ImGui::EndTable();
    }
    
    // ͳ����Ϣ
    ImGui::Separator();
    ImGui::Text(u8"�ܼ�: %zu ���ص�  |  ��ǰ����: %s", 
                ctx_->callbackUiVec.size(), 
                GetCallbackTypeName((CALLBACK_TYPE)ctx_->selectedCallbackType_));
    
    ImGui::EndGroup();
}

const char* CallbackWnd::GetCallbackTypeName(CALLBACK_TYPE type) {
    switch (type) {
        case TypeCreateProcess:     return u8"CreateProcess";
        case TypeCreateProcessEx:   return u8"CreateProcessEx";
        case TypeCreateThread:      return u8"CreateThread";
        case TypeLoadImage:         return u8"LoadImage";
        case TypeRegistry:          return u8"Registry";
        case TypeObject:            return u8"Object";
        case TypeBugCheck:          return u8"BugCheck";
        case TypeShutdown:          return u8"Shutdown";
        default:                    return u8"δ֪����";
    }
}

const char* CallbackWnd::GetCallbackTypeDesc(CALLBACK_TYPE type) {
    switch (type) {
        case TypeCreateProcess:     return u8"��ؽ��̴�������ֹ�¼�";
        case TypeCreateProcessEx:   return u8"��ؽ��̴�������ֹ�¼�(��չ��Ϣ)";
        case TypeCreateThread:      return u8"����̴߳�������ֹ�¼�";
        case TypeLoadImage:         return u8"���DLL/EXEģ������¼�";
        case TypeRegistry:          return u8"���ע�������¼�";
        case TypeObject:            return u8"��ض�������¼�";
        case TypeBugCheck:          return u8"ϵͳ����ʱ����";
        case TypeShutdown:          return u8"ϵͳ�ػ�ʱ����";
        default:                    return u8"δ֪�ص�����";
    }
}
