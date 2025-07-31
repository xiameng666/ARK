// FileManager.h
#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

struct FileItem {
    std::string fileName;          // 文件名
    std::string fullPath;          // 完整路径  
    LARGE_INTEGER fileSize;        // 文件大小
    DWORD attributes;              // 文件属性
    FILETIME createTime;           // 创建时间
    FILETIME modifyTime;           // 修改时间
    bool isDirectory;              // 是否目录
    std::string fileType;          // 文件类型(扩展名)
};

struct DirectoryNode {
    std::string path;                   // 目录路径
    std::string name;                   // 目录名
    std::vector<DirectoryNode> subDirs; // 子目录
    std::vector<FileItem> files;        // 文件列表
    bool isLoaded = false;              // 是否已加载
};

class FileManager {
public:
    FileManager();
    ~FileManager();

    // 核心功能
    bool LoadDrives();                                          // 加载驱动器列表
    bool LoadDirectory(const std::string& path);                // 加载指定目录
    bool RefreshDirectory(const std::string& path);             // 刷新目录

    // 数据获取
    std::vector<std::string> GetDriveList() const;              // 获取驱动器列表
    std::vector<FileItem> GetFiles(const std::string& path);    // 获取文件列表
    std::vector<DirectoryNode> GetSubDirectories(const std::string& path); // 获取子目录

    // 文件操作
    bool MyDeleteFile(const std::string& filePath);             // 删除文件
    bool MyCopyFile(const std::string& src, const std::string& dst); // 复制文件
    bool MyMoveFile(const std::string& src, const std::string& dst); // 移动文件

    // 工具函数
    std::string FormatFileSize(LARGE_INTEGER size);             // 格式化文件大小
    std::string FormatFileTime(FILETIME time);                  // 格式化时间
    std::string GetFileTypeFromExtension(const std::string& fileName); // 获取文件类型

private:
    std::map<std::string, DirectoryNode> directoryCache_;       // 目录缓存
    std::vector<std::string> driveList_;                        // 驱动器列表

    bool EnumDirectory(const std::string& path, DirectoryNode& node);
    bool IsValidPath(const std::string& path);
    
    // 内部转换函数
    static std::string WStringToUTF8(const std::wstring& wstr);
    static std::wstring UTF8ToWString(const std::string& str);
};