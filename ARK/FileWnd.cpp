#include "FileWnd.h"
#include <algorithm>

FileWnd::FileWnd(Context* ctx) : ImguiWnd(ctx) {
    currentFiles_ = fileManager_.GetFiles(currentPath_);
    currentDirs_ = fileManager_.GetSubDirectories(currentPath_);
}

void FileWnd::Render(bool* p_open) {
    ImGui::Begin(u8"文件浏览器", p_open, ImGuiWindowFlags_MenuBar);

    // 菜单栏
    if (ImGui::BeginMenuBar()) {
        if (ImGui::Button(u8"刷新")) {
            FlushCurrentDirectory();
        }
        ImGui::SameLine();

        // 路径显示
        ImGui::Text(u8"路径: %s", currentPath_.c_str());

        ImGui::EndMenuBar();
    }

    ImGui::Columns(2, "FileExplorer", true);

    // 左侧目录树
    RenderDirectoryTree();

    ImGui::NextColumn();

    // 右侧文件列表
    RenderFileList();

    ImGui::Columns(1);
    ImGui::End();
}

void FileWnd::RenderDirectoryTree() {
    ImGui::BeginChild("DirectoryTree", ImVec2(0, 0), true);
    ImGui::Text(u8"目录树");
    ImGui::Separator();

    // 显示驱动器列表
    auto drives = fileManager_.GetDriveList();
    for (const auto& drive : drives) {
        ImGui::PushID(drive.c_str());
        bool isExpanded = ImGui::TreeNodeEx(drive.c_str(), ImGuiTreeNodeFlags_OpenOnArrow);

        // 检查是否点击了驱动器文本（不是展开箭头）                                              
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
        // 检查字符串安全性 - 防止内存访问冲突
        if (dir.name.empty() || dir.path.empty()) {
            continue;
        }
        
        const char* displayName = dir.name.c_str();
        bool isExpanded = ImGui::TreeNodeEx(displayName, ImGuiTreeNodeFlags_OpenOnArrow);

        // 检查是否点击了目录文本（不是展开箭头）                                               
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            ChangeDirectory(dir.path);
        }

        if (isExpanded) {
            // 递归显示子目录                                                                   
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
    ImGui::Text(u8"文件列表");
    ImGui::Separator();

    // 搜索过滤器
    ImGui::InputText(u8"搜索", fileSearchFilter_, sizeof(fileSearchFilter_));
    ImGui::Separator();

    // 文件列表表格
    if (ImGui::BeginTable("FileTable", 4, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn(u8"名称", ImGuiTableColumnFlags_WidthFixed, 200);
        ImGui::TableSetupColumn(u8"大小", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableSetupColumn(u8"类型", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableSetupColumn(u8"修改时间", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        // 显示上级目录项
        if (currentPath_ != "C:\\" && currentPath_.length() > 3) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Selectable("..")) {
                NavigateUp();
            }
            ImGui::TableNextColumn();
            ImGui::Text("<DIR>");
            ImGui::TableNextColumn();
            ImGui::Text(u8"文件夹");
            ImGui::TableNextColumn();
            ImGui::Text("");
        }

        // 显示目录
        for (const auto& dir : currentDirs_) {
            // 应用搜索过滤器
            if (strlen(fileSearchFilter_) > 0) {
                std::string filter(fileSearchFilter_);
                std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);
                std::string name = dir.name;
                std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                if (name.find(filter) == std::string::npos) {
                    continue;
                }
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            // 严格的安全检查 - DirectoryNode对象验证
            try {
                // 检查对象是否有效
                if (dir.name.empty() || dir.path.empty()) {
                    continue;
                }
                
                // 创建字符串副本避免悬挂指针
                std::string safeName = dir.name;
                std::string safePath = dir.path;
                
                if (ImGui::Selectable(safeName.c_str())) {
                    ChangeDirectory(safePath);
                }
            } catch (...) {
                // 对象损坏时跳过
                continue;
            }

            ImGui::TableNextColumn();
            ImGui::Text("<DIR>");
            ImGui::TableNextColumn();
            ImGui::Text(u8"文件夹");
            ImGui::TableNextColumn();
            ImGui::Text("");
        }

        // 显示文件
        for (int i = 0; i < currentFiles_.size(); i++) {
            const auto& file = currentFiles_[i];

            // 应用搜索过滤器
            if (strlen(fileSearchFilter_) > 0) {
                std::string filter(fileSearchFilter_);
                std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);
                std::string name = file.fileName;
                std::transform(name.begin(), name.end(), name.begin(), ::tolower);
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
    // 清空当前数据，避免使用失效的引用
    currentFiles_.clear();
    currentDirs_.clear();
    
    fileManager_.RefreshDirectory(currentPath_);
    
    // 重新获取数据
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
