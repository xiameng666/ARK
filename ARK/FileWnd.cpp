#include "FileWnd.h"
#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <algorithm>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "shlwapi.lib")

FileWnd::FileWnd(Context* ctx) : ctx_(ctx), selectedIndex_(-1), showHiddenFiles_(false), showSystemFiles_(false) {
    currentPath_ = L"C:\\";
    strcpy_s(pathBuffer_, "C:\\");
    RefreshFileList();
}

FileWnd::~FileWnd() {
}

void FileWnd::Render() {
    if (!ImGui::Begin("文件管理器", nullptr, ImGuiWindowFlags_MenuBar)) {
        ImGui::End();
        return;
    }

    // 菜单栏
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("文件")) {
            if (ImGui::MenuItem("新建文件夹")) {
                CreateDirectoryW((currentPath_ + L"\\新建文件夹").c_str());
                RefreshFileList();
            }
            if (ImGui::MenuItem("刷新")) {
                RefreshFileList();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("退出")) {
                // 退出逻辑
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("查看")) {
            ImGui::MenuItem("显示隐藏文件", nullptr, &showHiddenFiles_);
            ImGui::MenuItem("显示系统文件", nullptr, &showSystemFiles_);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("工具")) {
            if (ImGui::MenuItem("文件解锁")) {
                if (selectedIndex_ >= 0 && selectedIndex_ < fileList_.size()) {
                    UnlockFile(fileList_[selectedIndex_].filePath);
                }
            }
            if (ImGui::MenuItem("文件粉碎")) {
                if (selectedIndex_ >= 0 && selectedIndex_ < fileList_.size()) {
                    ShredFile(fileList_[selectedIndex_].filePath);
                }
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
    }

    // 路径栏
    if (ImGui::InputText("路径", pathBuffer_, sizeof(pathBuffer_), ImGuiInputTextFlags_EnterReturnsTrue)) {
        currentPath_ = std::wstring(pathBuffer_, pathBuffer_ + strlen(pathBuffer_));
        NavigateToDirectory(currentPath_);
    }

    // 工具栏
    if (ImGui::Button("向上")) {
        size_t pos = currentPath_.find_last_of(L"\\");
        if (pos != std::wstring::npos && pos > 0) {
            currentPath_ = currentPath_.substr(0, pos);
            NavigateToDirectory(currentPath_);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("刷新")) {
        RefreshFileList();
    }
    ImGui::SameLine();
    if (ImGui::Button("删除") && selectedIndex_ >= 0) {
        if (selectedIndex_ < fileList_.size()) {
            DeleteFile(fileList_[selectedIndex_].filePath);
            RefreshFileList();
        }
    }

    ImGui::Separator();

    // 文件列表
    if (ImGui::BeginTable("文件列表", 5, ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBodyUntilResize)) {
        ImGui::TableSetupColumn("名称", ImGuiTableColumnFlags_DefaultSort);
        ImGui::TableSetupColumn("大小", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("类型", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("修改日期", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("属性", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow();

        for (int row = 0; row < fileList_.size(); row++) {
            const auto& file = fileList_[row];
            
            ImGui::TableNextRow();
            
            // 名称列
            ImGui::TableSetColumnIndex(0);
            char selectableId[256];
            sprintf_s(selectableId, "%s##%d", 
                file.isDirectory ? "[DIR] " : "", row);
            
            bool is_selected = (selectedIndex_ == row);
            if (ImGui::Selectable(selectableId, is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                selectedIndex_ = row;
            }
            
            // 双击进入目录
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                if (file.isDirectory) {
                    NavigateToDirectory(file.filePath);
                } else {
                    // 打开文件
                    ShellExecuteW(NULL, L"open", file.filePath.c_str(), NULL, NULL, SW_SHOW);
                }
            }
            
            // 显示文件名
            ImGui::SameLine();
            std::string fileNameStr(file.fileName.begin(), file.fileName.end());
            ImGui::Text("%s", fileNameStr.c_str());
            
            // 大小列
            ImGui::TableSetColumnIndex(1);
            std::string fileSizeStr(file.fileSize.begin(), file.fileSize.end());
            ImGui::Text("%s", fileSizeStr.c_str());
            
            // 类型列
            ImGui::TableSetColumnIndex(2);
            std::string fileTypeStr(file.fileType.begin(), file.fileType.end());
            ImGui::Text("%s", fileTypeStr.c_str());
            
            // 修改日期列
            ImGui::TableSetColumnIndex(3);
            std::string lastModifiedStr(file.lastModified.begin(), file.lastModified.end());
            ImGui::Text("%s", lastModifiedStr.c_str());
            
            // 属性列
            ImGui::TableSetColumnIndex(4);
            std::string attrs;
            if (file.attributes & FILE_ATTRIBUTE_READONLY) attrs += "R";
            if (file.attributes & FILE_ATTRIBUTE_HIDDEN) attrs += "H";
            if (file.attributes & FILE_ATTRIBUTE_SYSTEM) attrs += "S";
            if (file.attributes & FILE_ATTRIBUTE_ARCHIVE) attrs += "A";
            ImGui::Text("%s", attrs.c_str());
        }
        
        ImGui::EndTable();
    }

    // 状态栏
    ImGui::Separator();
    ImGui::Text("选中: %d 个文件", selectedIndex_ >= 0 ? 1 : 0);
    ImGui::SameLine();
    ImGui::Text("总计: %zu 个项目", fileList_.size());

    ImGui::End();
}

void FileWnd::Flush() {
    RefreshFileList();
}

void FileWnd::RefreshFileList() {
    fileList_.clear();
    
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW((currentPath_ + L"\\*").c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            // 跳过 . 和 ..
            if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0) {
                continue;
            }
            
            // 过滤隐藏文件和系统文件
            if (!showHiddenFiles_ && (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {
                continue;
            }
            if (!showSystemFiles_ && (findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
                continue;
            }
            
            FILE_INFO fileInfo;
            fileInfo.fileName = findData.cFileName;
            fileInfo.filePath = currentPath_ + L"\\" + findData.cFileName;
            fileInfo.attributes = findData.dwFileAttributes;
            fileInfo.isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            fileInfo.isSelected = false;
            
            if (fileInfo.isDirectory) {
                fileInfo.fileSize = L"<DIR>";
                fileInfo.size = 0;
            } else {
                ULARGE_INTEGER fileSize;
                fileSize.LowPart = findData.nFileSizeLow;
                fileSize.HighPart = findData.nFileSizeHigh;
                fileInfo.size = fileSize.QuadPart;
                fileInfo.fileSize = FormatFileSize(fileSize.QuadPart);
            }
            
            fileInfo.fileType = GetFileType(findData.cFileName);
            fileInfo.lastModified = FormatDateTime(findData.ftLastWriteTime);
            
            fileList_.push_back(fileInfo);
            
        } while (FindNextFileW(hFind, &findData));
        
        FindClose(hFind);
    }
    
    SortFileList();
}

void FileWnd::NavigateToDirectory(const std::wstring& path) {
    currentPath_ = path;
    strcpy_s(pathBuffer_, std::string(path.begin(), path.end()).c_str());
    RefreshFileList();
}

std::wstring FileWnd::FormatFileSize(ULONGLONG size) {
    if (size < 1024) {
        return std::to_wstring(size) + L" B";
    } else if (size < 1024 * 1024) {
        return std::to_wstring(size / 1024) + L" KB";
    } else if (size < 1024 * 1024 * 1024) {
        return std::to_wstring(size / (1024 * 1024)) + L" MB";
    } else {
        return std::to_wstring(size / (1024 * 1024 * 1024)) + L" GB";
    }
}

std::wstring FileWnd::GetFileType(const std::wstring& fileName) {
    size_t pos = fileName.find_last_of(L".");
    if (pos == std::wstring::npos) {
        return L"文件";
    }
    
    std::wstring ext = fileName.substr(pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towupper);
    
    if (ext == L"EXE") return L"应用程序";
    if (ext == L"DLL") return L"动态链接库";
    if (ext == L"TXT") return L"文本文档";
    if (ext == L"DOC" || ext == L"DOCX") return L"Word文档";
    if (ext == L"XLS" || ext == L"XLSX") return L"Excel文档";
    if (ext == L"JPG" || ext == L"JPEG" || ext == L"PNG" || ext == L"GIF") return L"图像文件";
    if (ext == L"MP3" || ext == L"WAV" || ext == L"FLAC") return L"音频文件";
    if (ext == L"MP4" || ext == L"AVI" || ext == L"MKV") return L"视频文件";
    
    return ext + L"文件";
}

std::wstring FileWnd::FormatDateTime(const FILETIME& ft) {
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);
    
    wchar_t buffer[64];
    swprintf_s(buffer, L"%04d-%02d-%02d %02d:%02d", 
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
    
    return std::wstring(buffer);
}

void FileWnd::SortFileList() {
    std::sort(fileList_.begin(), fileList_.end(), 
        [](const FILE_INFO& a, const FILE_INFO& b) {
            // 目录优先
            if (a.isDirectory != b.isDirectory) {
                return a.isDirectory > b.isDirectory;
            }
            // 按名称排序
            return a.fileName < b.fileName;
        });
}

bool FileWnd::DeleteFile(const std::wstring& filePath) {
    if (::DeleteFileW(filePath.c_str())) {
        ctx_->arkR3.Log("成功删除文件: %s\n", std::string(filePath.begin(), filePath.end()).c_str());
        return true;
    } else {
        DWORD error = GetLastError();
        ctx_->arkR3.Log("删除文件失败: %s (错误码: %d)\n", std::string(filePath.begin(), filePath.end()).c_str(), error);
        return false;
    }
}

bool FileWnd::UnlockFile(const std::wstring& filePath) {
    // 这里需要调用内核驱动来解锁文件
    // 暂时用用户态API尝试
    ctx_->arkR3.Log("尝试解锁文件: %s\n", std::string(filePath.begin(), filePath.end()).c_str());
    
    // 可以通过驱动通信来调用内核解锁函数
    // 这里只是示例，实际需要实现驱动通信
    return true;
}

bool FileWnd::ShredFile(const std::wstring& filePath) {
    ctx_->arkR3.Log("尝试粉碎文件: %s\n", std::string(filePath.begin(), filePath.end()).c_str());
    
    // 文件粉碎需要：
    // 1. 先解锁文件
    // 2. 多次覆盖文件内容
    // 3. 删除文件
    
    if (UnlockFile(filePath)) {
        // 这里实现文件覆盖逻辑
        // 实际需要驱动支持
        return DeleteFile(filePath);
    }
    
    return false;
}

void FileWnd::SetCurrentPath(const std::wstring& path) {
    currentPath_ = path;
    strcpy_s(pathBuffer_, std::string(path.begin(), path.end()).c_str());
    RefreshFileList();
}

bool FileWnd::ExecuteFileOperation(int operation, const std::wstring& filePath) {
    switch (operation) {
        case 0: // 删除
            return DeleteFile(filePath);
        case 1: // 解锁
            return UnlockFile(filePath);
        case 2: // 粉碎
            return ShredFile(filePath);
        default:
            return false;
    }
}

void FileWnd::ShowFileProperties(const std::wstring& filePath) {
    SHELLEXECUTEINFOW sei = {0};
    sei.cbSize = sizeof(sei);
    sei.lpVerb = L"properties";
    sei.lpFile = filePath.c_str();
    sei.nShow = SW_SHOW;
    sei.fMask = SEE_MASK_INVOKEIDLIST;
    
    ShellExecuteExW(&sei);
}

void FileWnd::SearchFiles(const std::wstring& searchPattern) {
    // 实现文件搜索功能
    ctx_->arkR3.Log("搜索文件: %s\n", std::string(searchPattern.begin(), searchPattern.end()).c_str());
}

bool FileWnd::RenameFile(const std::wstring& oldPath, const std::wstring& newName) {
    std::wstring newPath = currentPath_ + L"\\" + newName;
    if (::MoveFileW(oldPath.c_str(), newPath.c_str())) {
        ctx_->arkR3.Log("成功重命名文件: %s -> %s\n", 
            std::string(oldPath.begin(), oldPath.end()).c_str(),
            std::string(newName.begin(), newName.end()).c_str());
        return true;
    } else {
        DWORD error = GetLastError();
        ctx_->arkR3.Log("重命名文件失败: %s (错误码: %d)\n", 
            std::string(oldPath.begin(), oldPath.end()).c_str(), error);
        return false;
    }
}

bool FileWnd::CopyFile(const std::wstring& srcPath, const std::wstring& dstPath) {
    if (::CopyFileW(srcPath.c_str(), dstPath.c_str(), FALSE)) {
        ctx_->arkR3.Log("成功复制文件: %s -> %s\n", 
            std::string(srcPath.begin(), srcPath.end()).c_str(),
            std::string(dstPath.begin(), dstPath.end()).c_str());
        return true;
    } else {
        DWORD error = GetLastError();
        ctx_->arkR3.Log("复制文件失败: %s (错误码: %d)\n", 
            std::string(srcPath.begin(), srcPath.end()).c_str(), error);
        return false;
    }
}

bool FileWnd::MoveFile(const std::wstring& srcPath, const std::wstring& dstPath) {
    if (::MoveFileW(srcPath.c_str(), dstPath.c_str())) {
        ctx_->arkR3.Log("成功移动文件: %s -> %s\n", 
            std::string(srcPath.begin(), srcPath.end()).c_str(),
            std::string(dstPath.begin(), dstPath.end()).c_str());
        return true;
    } else {
        DWORD error = GetLastError();
        ctx_->arkR3.Log("移动文件失败: %s (错误码: %d)\n", 
            std::string(srcPath.begin(), srcPath.end()).c_str(), error);
        return false;
    }
}

bool FileWnd::CreateDirectory(const std::wstring& dirPath) {
    if (::CreateDirectoryW(dirPath.c_str(), NULL)) {
        ctx_->arkR3.Log("成功创建目录: %s\n", 
            std::string(dirPath.begin(), dirPath.end()).c_str());
        return true;
    } else {
        DWORD error = GetLastError();
        ctx_->arkR3.Log("创建目录失败: %s (错误码: %d)\n", 
            std::string(dirPath.begin(), dirPath.end()).c_str(), error);
        return false;
    }
} 