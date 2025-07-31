#include "FileWnd.h"
#include <algorithm>

FileWnd::FileWnd(Context* ctx) : ImguiWnd(ctx) {
    currentFiles_ = fileManager_.GetFiles(currentPath_);
    currentDirs_ = fileManager_.GetSubDirectories(currentPath_);
}

void FileWnd::Render(bool* p_open) {
    ImGui::Begin(u8"�ļ������", p_open, ImGuiWindowFlags_MenuBar);

    // �˵���
    if (ImGui::BeginMenuBar()) {
        if (ImGui::Button(u8"ˢ��")) {
            FlushCurrentDirectory();
        }
        ImGui::SameLine();

        // ·����ʾ
        ImGui::Text(u8"·��: %s", currentPath_.c_str());

        ImGui::EndMenuBar();
    }

    ImGui::Columns(2, "FileExplorer", true);

    // ���Ŀ¼��
    RenderDirectoryTree();

    ImGui::NextColumn();

    // �Ҳ��ļ��б�
    RenderFileList();

    ImGui::Columns(1);
    ImGui::End();
}

void FileWnd::RenderDirectoryTree() {
    ImGui::BeginChild("DirectoryTree", ImVec2(0, 0), true);
    ImGui::Text(u8"Ŀ¼��");
    ImGui::Separator();

    // ��ʾ����ѡ��
    ImGui::PushID("Desktop");
    ImGui::TreeNodeEx(u8"����", ImGuiTreeNodeFlags_OpenOnArrow);
    
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        // ��ȡ����·��
        char desktopPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, 0, desktopPath))) {
            ChangeDirectory(desktopPath);
        }
    }
    
    ImGui::PopID();
    
    ImGui::Separator();

    // ��ʾ�������б�
    auto drives = fileManager_.GetDriveList();
    for (const auto& drive : drives) {
        ImGui::PushID(drive.c_str());
        bool isExpanded = ImGui::TreeNodeEx(drive.c_str(), ImGuiTreeNodeFlags_OpenOnArrow);

        // ����Ƿ������������ı�������չ����ͷ��                                              
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            ChangeDirectory(drive);
        }

        if (isExpanded) {
            RenderDirectoryNode(drive);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    ImGui::EndChild();
}

void FileWnd::RenderDirectoryNode(const std::string& path) {
    auto subDirs = fileManager_.GetSubDirectories(path);

    for (const auto& dir : subDirs) {
        // ����ַ�����ȫ�� - ��ֹ�ڴ���ʳ�ͻ
        if (dir.name.empty() || dir.path.empty()) {
            continue;
        }
        
        const char* displayName = dir.name.c_str();
        bool isExpanded = ImGui::TreeNodeEx(displayName, ImGuiTreeNodeFlags_OpenOnArrow);

        // ����Ƿ�����Ŀ¼�ı�������չ����ͷ��                                               
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            ChangeDirectory(dir.path);
        }

        if (isExpanded) {
            // �ݹ���ʾ��Ŀ¼                                                                   
            RenderDirectoryNode(dir.path);
            ImGui::TreePop();           
        }

        if (ImGui::IsItemClicked()) {
            ChangeDirectory(dir.path);
        }
    }
}

void FileWnd::RenderFileList() {
    ImGui::BeginChild("FileList", ImVec2(0, 0), true);
    ImGui::Text(u8"�ļ��б�");
    ImGui::Separator();

    // ����������
    ImGui::InputText(u8"����", fileSearchFilter_, sizeof(fileSearchFilter_));
    ImGui::Separator();

    // �ļ��б���
    if (ImGui::BeginTable("FileTable", 4, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn(u8"����", ImGuiTableColumnFlags_WidthFixed, 200);
        ImGui::TableSetupColumn(u8"��С", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableSetupColumn(u8"����", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableSetupColumn(u8"�޸�ʱ��", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        // ��ʾ�ϼ�Ŀ¼��
        if (currentPath_ != "C:\\" && currentPath_.length() > 3) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Selectable("..")) {
                NavigateUp();
            }
            ImGui::TableNextColumn();
            ImGui::Text("<DIR>");
            ImGui::TableNextColumn();
            ImGui::Text(u8"�ļ���");
            ImGui::TableNextColumn();
            ImGui::Text("");
        }

        // ��ʾĿ¼
        for (const auto& dir : currentDirs_) {
            // Ӧ������������
            if (strlen(fileSearchFilter_) > 0) {
                std::string filter(fileSearchFilter_);
                std::string name = dir.name;

                // ת��ΪСд���бȽ�
                for (char& c : name) c = (char)tolower(c);
                for (char& c : filter) c = (char)tolower(c);

                if (name.find(filter) == std::string::npos) {
                    continue;
                }
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            try {
                if (dir.name.empty() || dir.path.empty()) {
                    continue;
                }
                
                // �����ַ���������������ָ��
                std::string safeName = dir.name;
                std::string safePath = dir.path;
                
                 if (ImGui::Selectable(safeName.c_str())) {
                     ChangeDirectory(safePath);
                 }
                 
                 // �Ҽ��˵�
                 if (ImGui::BeginPopupContextItem(("dir_context_" + safeName).c_str())) {
                     ShowDirectoryContextMenu(dir);
                     ImGui::EndPopup();
                 }
            } catch (...) {
                // ������ʱ����
                continue;
            }

            ImGui::TableNextColumn();
            ImGui::Text("<DIR>");
            ImGui::TableNextColumn();
            ImGui::Text(u8"�ļ���");
            ImGui::TableNextColumn();
            ImGui::Text("");
        }

        // ��ʾ�ļ�
        for (int i = 0; i < currentFiles_.size(); i++) {
            const auto& file = currentFiles_[i];

            // Ӧ������������
            if (strlen(fileSearchFilter_) > 0) {
                std::string filter(fileSearchFilter_);
                std::string name = file.fileName;

                // ת��ΪСд���бȽ�
                for (char& c : name) c = (char)tolower(c);
                for (char& c : filter) c = (char)tolower(c);

                if (name.find(filter) == std::string::npos) {
                    continue;
                }
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

                         bool isSelected = (selectedFileIndex_ == i);
             if (ImGui::Selectable(file.fileName.c_str(), isSelected)) {
                 selectedFileIndex_ = i;
             }
             
             // �Ҽ��˵�
             if (ImGui::BeginPopupContextItem(("file_context_" + std::to_string(i)).c_str())) {
                 ShowFileContextMenu(file);
                 ImGui::EndPopup();
             }

            ImGui::TableNextColumn();
            ImGui::Text("%s", fileManager_.FormatFileSize(file.fileSize).c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%s", file.fileType.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%s", fileManager_.FormatFileTime(file.modifyTime).c_str());
        }

        ImGui::EndTable();
    }

    ImGui::EndChild();
}

void FileWnd::ChangeDirectory(const std::string& newPath) {
    currentPath_ = newPath;
    FlushCurrentDirectory();
}

void FileWnd::FlushCurrentDirectory() {
    // ��յ�ǰ���ݣ�����ʹ��ʧЧ������
    currentFiles_.clear();
    currentDirs_.clear();
    
    fileManager_.RefreshDirectory(currentPath_);
    
    // ���»�ȡ
    currentFiles_ = fileManager_.GetFiles(currentPath_);
    currentDirs_ = fileManager_.GetSubDirectories(currentPath_);
    
    selectedFileIndex_ = -1;
}

void FileWnd::NavigateUp() {
    size_t pos = currentPath_.find_last_of('\\');
    if (pos != std::string::npos ) {
        std::string parentPath = currentPath_.substr(0, pos);
        ChangeDirectory(parentPath);
    }
}

void FileWnd::ShowFileContextMenu(const FileItem& file) {
    std::string fullPath = currentPath_ + "\\" + file.fileName;
    
    if (ImGui::MenuItem(u8"����·��")) {
        ImGui::SetClipboardText(fullPath.c_str());
    }
    
    ImGui::Separator();
    
    if (ImGui::MenuItem(u8"���ռ��")) {

        ctx_->arkR3.Log(u8"���Խ���ļ�ռ��: %s\n", fullPath.c_str());
        // ctx_->arkR3.UnlockFile(fullPath);
    }
    
    if (ImGui::MenuItem(u8"�����ļ�")) {

        ctx_->arkR3.Log(u8"���Է����ļ�: %s\n", fullPath.c_str());
        // ctx_->arkR3.ShredFile(fullPath);
        FlushCurrentDirectory(); // ˢ���б�
    }
}

void FileWnd::ShowDirectoryContextMenu(const DirectoryNode& dir) {

    if (ImGui::MenuItem(u8"����·��")) {
        ImGui::SetClipboardText(dir.path.c_str());
    }
    
    ImGui::Separator();
    
    if (ImGui::MenuItem(u8"����Ŀ¼")) {

        ctx_->arkR3.Log(u8"���Է���Ŀ¼: %s\n", dir.path.c_str());
        // ctx_->arkR3.ShredDirectory(dir.path);
    }
}


