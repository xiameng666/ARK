// 重构后的FileManager.h (纯UTF-8设计)
#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <map>

struct FileItem {
    std::string fileName;          // 文件名 (UTF-8)
    std::string fullPath;          // 完整路径 (UTF-8)
    LARGE_INTEGER fileSize;        // 文件大小
    DWORD attributes;              // 文件属性
    FILETIME createTime;           // 创建时间
    FILETIME modifyTime;           // 修改时间
    bool isDirectory;              // 是否目录
    std::string fileType;          // 文件类型 (UTF-8)
};

struct DirectoryNode {
    std::string path;                       // 目录路径 (UTF-8)
    std::string name;                       // 目录名 (UTF-8)
    std::vector<DirectoryNode> subDirs;     // 子目录
    std::vector<FileItem> files;            // 文件列表
    bool isLoaded = false;                  // 是否已加载
};

class FileManager {
public:
    // 核心功能
    bool LoadDrives();
    bool LoadDirectory(const std::string& path);
    bool RefreshDirectory(const std::string& path);

    // 数据获取 (直接返回UTF-8字符串)
    std::vector<std::string> GetDriveList() const;
    std::vector<FileItem> GetFiles(const std::string& path);
    std::vector<DirectoryNode> GetSubDirectories(const std::string& path);

    // 工具函数
    std::string FormatFileSize(LARGE_INTEGER size);
    std::string FormatFileTime(FILETIME time);
    std::string GetFileTypeFromExtension(const std::string& fileName);

private:
    std::map<std::string, DirectoryNode> directoryCache_;
    std::vector<std::string> driveList_;
    
    bool EnumDirectory(const std::string& path, DirectoryNode& node);
    bool IsValidPath(const std::string& path);
    
    // 内部转换函数
    static std::string WStringToUTF8(const std::wstring& wstr);
    static std::wstring UTF8ToWString(const std::string& str);
};