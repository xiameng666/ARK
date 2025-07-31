# Windows File Explorer Component

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-Windows-blue.svg)](https://www.microsoft.com/windows)
[![Language](https://img.shields.io/badge/Language-C%2B%2B17-orange.svg)](https://isocpp.org/)

> High-performance file explorer component built with ImGui and Windows API, C++17

English | [中文](README.md)

## 📖 Overview

This project is a modern Windows file explorer component built using the ImGui immediate mode GUI framework. Created while developing the ARK toolkit when a file system component was needed, it provides an intuitive file system browsing interface with tree-structured directory navigation and detailed file listings.

![image](image.png)

## 🏗️ Architecture

### Component Design

```
┌─────────────────┐    ┌──────────────────┐
│   FileWnd       │────│   FileManager    │
│  (UI Layer)     │    │  (Data Layer)    │
└─────────────────┘    └──────────────────┘
         │                       │
         ▼                       ▼
┌─────────────────┐    ┌──────────────────┐
│   ImGui API     │    │  Windows API     │
│   (GUI Framework)│    │  (File System)   │
└─────────────────┘    └──────────────────┘
```

### Core Class Structure

#### FileManager (File Management)
```cpp
class FileManager {
    // Core functionality
    bool LoadDrives();                    // Load drive list
    bool LoadDirectory(const string& path); // Load specified directory
    vector<FileItem> GetFiles(const string& path); // Get file list
    
    // Utility functions
    string FormatFileSize(LARGE_INTEGER size);     // Format file size
    string FormatFileTime(FILETIME time);          // Format time
    
private:
    map<string, DirectoryNode> directoryCache_;    // Directory cache
    vector<string> driveList_;                     // Drive list
};
```

#### FileWnd (File Window)
```cpp
class FileWnd : public ImguiWnd {
    void Render(bool* p_open = nullptr);           // Render main interface
    void RenderDirectoryTree();                    // Render directory tree
    void RenderFileList();                         // Render file list
    
private:
    FileManager fileManager_;                      // File manager instance
    string currentPath_;                           // Current path
    vector<FileItem> currentFiles_;                // Current file list
};
```

### Data Structures

#### FileItem (File Entry)
```cpp
struct FileItem {
    string fileName;          // File name
    string fullPath;          // Full path
    LARGE_INTEGER fileSize;   // File size
    DWORD attributes;         // File attributes
    FILETIME createTime;      // Creation time
    FILETIME modifyTime;      // Modification time
    bool isDirectory;         // Is directory
    string fileType;          // File type
};
```

## 🚀 Usage

### Integration into Existing Projects

```cpp
// 1. Include headers
#include "FileWnd.h"

// 2. Add member to application class
class YourApp {
private:
    FileWnd fileWnd_;
    bool showFileExplorer_ = false;
};

// 3. Call in render loop
void YourApp::Render() {
    if (showFileExplorer_) {
        fileWnd_.Render(&showFileExplorer_);
    }
}
```

### Interactive Operations

#### Directory Tree Operations
- **Click Arrow** ▶️ Expand/collapse subdirectories
- **Click Text** 📁 Display directory contents in right panel
- **Auto Loading** 🔄 Load subdirectories on demand

#### File List Operations
- **Single Click** 🖱️ Highlight selected file
- **Double Click Directory** 📂 Enter subdirectory
- **Click ".."** ⬆️ Navigate to parent directory
- **Search Filter** 🔍 Real-time filename filtering

## 📚 API Documentation

### FileManager API

| Method | Description | Return Value |
|--------|-------------|-------------|
| `LoadDrives()` | Load system drive list | `bool` - Success/Failure |
| `LoadDirectory(path)` | Load specified directory content | `bool` - Success/Failure |
| `GetFiles(path)` | Get file list in directory | `vector<FileItem>` |
| `GetSubDirectories(path)` | Get subdirectory list | `vector<DirectoryNode>` |
| `FormatFileSize(size)` | Format file size display | `string` - e.g. "1.5 MB" |
| `FormatFileTime(time)` | Format time display | `string` - e.g. "2024-01-15 14:30" |

### FileWnd API

| Method | Description | Parameters |
|--------|-------------|------------|
| `Render(p_open)` | Render file explorer window | `bool*` - Window open state |
| `ChangeDirectory(path)` | Change to specified directory | `const string&` - Target path |
| `FlushCurrentDirectory()` | Refresh current directory content | None |
| `NavigateUp()` | Navigate to parent directory | None |

---

⭐ If this project helps you, please give it a Star!