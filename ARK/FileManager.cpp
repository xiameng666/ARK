#include "FileManager.h"

FileManager::FileManager() {
    LoadDrives();
}

FileManager::~FileManager() {
    directoryCache_.clear();
}

bool FileManager::LoadDrives() {
    driveList_.clear();
    
    DWORD drives = GetLogicalDrives();
    for (int i = 0; i < 26; i++) {
        if (drives & (1 << i)) {
            std::string drive;
            drive += (char)('A' + i);
            drive += ":\\";
            driveList_.push_back(drive);
        }
    }
    return !driveList_.empty();
}

bool FileManager::LoadDirectory(const std::string& path) {
    if (!IsValidPath(path)) {
        return false;
    }
    
    if (directoryCache_.find(path) != directoryCache_.end() && 
        directoryCache_[path].isLoaded) {
        return true;
    }
    
    DirectoryNode node;
    node.path = path;
    size_t pos = path.find_last_of("\\");
    if (pos != std::string::npos) {
        node.name = path.substr(pos + 1);
    } else {
        node.name = path;
    }
    
    if (EnumDirectory(path, node)) {
        node.isLoaded = true;
        directoryCache_[path] = node;
        return true;
    }
    
    return false;
}

bool FileManager::RefreshDirectory(const std::string& path) {
    directoryCache_.erase(path);
    return LoadDirectory(path);
}

std::vector<std::string> FileManager::GetDriveList() const {
    return driveList_;
}

std::vector<FileItem> FileManager::GetFiles(const std::string& path) {
    if (LoadDirectory(path)) {
        auto it = directoryCache_.find(path);
        if (it != directoryCache_.end()) {
            return it->second.files;
        }
    }
    return std::vector<FileItem>();
}

std::vector<DirectoryNode> FileManager::GetSubDirectories(const std::string& path) {
    if (LoadDirectory(path)) {
        auto it = directoryCache_.find(path);
        if (it != directoryCache_.end()) {

            std::vector<DirectoryNode> result;
            for (const auto& dir : it->second.subDirs) {
                // 只返回有效的DirectoryNode对象
                if (!dir.name.empty() && !dir.path.empty()) {
                    result.push_back(dir);
                }
            }
            return result;
        }
    }
    return std::vector<DirectoryNode>();
}

bool FileManager::MyDeleteFile(const std::string& filePath) {
    std::wstring wpath = UTF8ToWString(filePath);
    return ::DeleteFileW(wpath.c_str()) != 0;
}

bool FileManager::MyCopyFile(const std::string& src, const std::string& dst) {
    std::wstring wsrc = UTF8ToWString(src);
    std::wstring wdst = UTF8ToWString(dst);
    return ::CopyFileW(wsrc.c_str(), wdst.c_str(), FALSE) != 0;
}

bool FileManager::MyMoveFile(const std::string& src, const std::string& dst) {
    std::wstring wsrc = UTF8ToWString(src);
    std::wstring wdst = UTF8ToWString(dst);
    return ::MoveFileW(wsrc.c_str(), wdst.c_str()) != 0;
}

std::string FileManager::FormatFileSize(LARGE_INTEGER size) {
    const char* units[] = { "B", "KB", "MB", "GB", "TB" };
    double fileSize = static_cast<double>(size.QuadPart);
    int unitIndex = 0;
    
    while (fileSize >= 1024.0 && unitIndex < 4) {
        fileSize /= 1024.0;
        unitIndex++;
    }
    
    std::stringstream ss;
    if (unitIndex == 0) {
        ss << static_cast<long long>(fileSize) << " " << units[unitIndex];
    } else {
        ss << std::fixed << std::setprecision(2) << fileSize << " " << units[unitIndex];
    }
    
    return ss.str();
}

std::string FileManager::FormatFileTime(FILETIME time) {
    SYSTEMTIME st;
    if (!FileTimeToSystemTime(&time, &st)) {
        return "Unknown";
    }
    
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << st.wYear << "-"
       << std::setw(2) << st.wMonth << "-"
       << std::setw(2) << st.wDay << " "
       << std::setw(2) << st.wHour << ":"
       << std::setw(2) << st.wMinute;
    
    return ss.str();
}

std::string FileManager::GetFileTypeFromExtension(const std::string& fileName) {
    size_t dotPos = fileName.find_last_of('.');
    if (dotPos != std::string::npos && dotPos < fileName.length() - 1) {
        std::string ext = fileName.substr(dotPos + 1);
        for (char& c : ext) {
            c = (char)toupper(c);
        }
        return ext;
    }
    return "";
}

bool FileManager::EnumDirectory(const std::string& path, DirectoryNode& node) {
    std::wstring wpath = UTF8ToWString(path);
    if (wpath.back() != L'\\') {
        wpath += L"\\";
    }
    wpath += L"*";
    
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(wpath.c_str(), &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    do {
        if (wcscmp(findData.cFileName, L".") == 0 || 
            wcscmp(findData.cFileName, L"..") == 0) {
            continue;
        }
        
        std::string fileName = WStringToUTF8(findData.cFileName);

        if (fileName.empty()) {
            continue;
        }
        
        std::string fullPath = path;
        if (fullPath.back() != '\\') {
            fullPath += "\\";
        }
        fullPath += fileName;
        
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            DirectoryNode subDir;
            subDir.name = fileName;
            subDir.path = fullPath;
            subDir.isLoaded = false;
            node.subDirs.push_back(subDir);
        } else {
            FileItem file;
            file.fileName = fileName;
            file.fullPath = fullPath;
            file.fileSize.HighPart = findData.nFileSizeHigh;
            file.fileSize.LowPart = findData.nFileSizeLow;
            file.attributes = findData.dwFileAttributes;
            file.createTime = findData.ftCreationTime;
            file.modifyTime = findData.ftLastWriteTime;
            file.isDirectory = false;
            file.fileType = GetFileTypeFromExtension(fileName);
            
            node.files.push_back(file);
        }
    } while (FindNextFileW(hFind, &findData));
    
    FindClose(hFind);
    return true;
}

bool FileManager::IsValidPath(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    
    std::wstring wpath = UTF8ToWString(path);
    DWORD attr = GetFileAttributesW(wpath.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

std::string FileManager::WStringToUTF8(const std::wstring& wstr) {
    if (wstr.empty()) {
        return std::string();
    }
    
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (sizeNeeded <= 0) {
        // 转换失败时
        return std::string("?");
    }
    
    std::string result(sizeNeeded - 1, 0);
    int converted = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], sizeNeeded, NULL, NULL);
    if (converted <= 0) {
        return std::string("?");
    }
    
    return result;
}

std::wstring FileManager::UTF8ToWString(const std::string& str) {
    if (str.empty()) {
        return std::wstring();
    }
    
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    if (sizeNeeded <= 0) {
        return std::wstring();
    }
    
    std::wstring result(sizeNeeded - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], sizeNeeded);
    return result;
}
