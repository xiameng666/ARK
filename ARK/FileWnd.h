#pragma once
#include "Interface.h"
#include "FileManager.h"

class FileWnd : public ImguiWnd {
public:
    explicit FileWnd(Context* ctx);
    void Render(bool* p_open = nullptr) override;

private:
    FileManager fileManager_;

    std::string currentPath_ = "C:\\";                     // 当前路径
    std::vector<FileItem> currentFiles_;                   // 当前文件列表
    std::vector<DirectoryNode> currentDirs_;               // 当前目录列表
    int selectedFileIndex_ = -1;                           // 选中文件索引
    char fileSearchFilter_[256] = "";                      // 文件搜索过滤器

    void RenderDirectoryTree();                            // 渲染目录树
    void RenderDirectoryNode(const std::string& path);     // 渲染目录节点
    void RenderFileList();                                 // 渲染文件列表
    
    void ChangeDirectory(const std::string& newPath);      // 切换目录
    void FlushCurrentDirectory();                          // 刷新当前目录
    void NavigateUp();                                     // 导航上级目录
};