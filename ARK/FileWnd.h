#pragma once
#include "Interface.h"
#include <vector>
#include <string>

// 文件信息结构体
struct FILE_INFO {
    std::wstring fileName;        // 文件名
    std::wstring filePath;        // 完整路径
    std::wstring fileSize;        // 文件大小（格式化显示）
    std::wstring fileType;        // 文件类型
    std::wstring lastModified;    // 最后修改时间
    DWORD attributes;             // 文件属性
    ULONGLONG size;               // 文件大小（字节）
    bool isDirectory;             // 是否为目录
    bool isSelected;              // 是否被选中
};

class FileWnd {
private:
    Context* ctx_;
    std::vector<FILE_INFO> fileList_;
    std::wstring currentPath_;
    int selectedIndex_;
    char pathBuffer_[MAX_PATH];
    bool showHiddenFiles_;
    bool showSystemFiles_;
    
    // 文件操作相关
    void RefreshFileList();
    void NavigateToDirectory(const std::wstring& path);
    void GetFileIcon(const std::wstring& fileName, const std::wstring& fileType);
    std::wstring FormatFileSize(ULONGLONG size);
    std::wstring GetFileType(const std::wstring& fileName);
    std::wstring FormatDateTime(const FILETIME& ft);
    void SortFileList();
    
    // 文件操作函数
    bool DeleteFile(const std::wstring& filePath);
    bool RenameFile(const std::wstring& oldPath, const std::wstring& newName);
    bool CopyFile(const std::wstring& srcPath, const std::wstring& dstPath);
    bool MoveFile(const std::wstring& srcPath, const std::wstring& dstPath);
    bool CreateDirectory(const std::wstring& dirPath);
    
    // 文件解锁/粉碎相关
    bool UnlockFile(const std::wstring& filePath);
    bool ShredFile(const std::wstring& filePath);
    bool ForceDeleteFile(const std::wstring& filePath);

public:
    FileWnd(Context* ctx);
    ~FileWnd();
    
    void Render();
    void Flush();
    
    // 公共接口
    void SetCurrentPath(const std::wstring& path);
    std::wstring GetCurrentPath() const { return currentPath_; }
    const std::vector<FILE_INFO>& GetFileList() const { return fileList_; }
    
    // 文件操作接口
    bool ExecuteFileOperation(int operation, const std::wstring& filePath);
    void ShowFileProperties(const std::wstring& filePath);
    void SearchFiles(const std::wstring& searchPattern);
}; 