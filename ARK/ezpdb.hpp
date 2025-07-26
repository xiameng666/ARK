#pragma once

#include <stdio.h>
#include <Windows.h>
#include <DbgHelp.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <locale>
#include <codecvt>
#include <Shlwapi.h>
#include <objbase.h>
#include <Urlmon.h>
#include <PathCch.h>
#include <WinInet.h>
#include <winhttp.h>

#pragma comment(lib, "DbgHelp.lib")
#pragma comment(lib, "Urlmon.lib")
#pragma comment(lib, "pathcch.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "OLE32.lib")
#pragma comment(lib, "WinInet.lib")
#pragma comment(lib, "winhttp.lib")
namespace ez
{
#pragma warning(push,1)
	namespace md5
	{

		// https://github.com/pod32g/MD5/blob/master/md5.c

		// Constants are the integer part of the sines of integers (in radians) * 2^32.
		const UINT32 k[64] = {
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee ,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501 ,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be ,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821 ,
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa ,
		0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8 ,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed ,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a ,
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c ,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70 ,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05 ,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665 ,
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039 ,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1 ,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1 ,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };

		// r specifies the per-round shift amounts
		const UINT32 r[] = { 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
							  5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
							  4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
							  6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21 };

		// leftrotate function definition
#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))

		inline void to_bytes(UINT32 val, UINT8* bytes)
		{
			bytes[0] = (UINT8)val;
			bytes[1] = (UINT8)(val >> 8);
			bytes[2] = (UINT8)(val >> 16);
			bytes[3] = (UINT8)(val >> 24);
		}

		inline UINT32 to_int32(const UINT8* bytes)
		{
			return (UINT32)bytes[0]
				| ((UINT32)bytes[1] << 8)
				| ((UINT32)bytes[2] << 16)
				| ((UINT32)bytes[3] << 24);
		}

		inline void md5(const UINT8* initial_msg, UINT32 initial_len, UINT8* digest) {

			// These vars will contain the hash
			UINT32 h0, h1, h2, h3;

			// Message (to prepare)
			UINT8* msg = NULL;

			UINT32 new_len, offset;
			UINT32 w[16];
			UINT32 a, b, c, d, i, f, g, temp;

			// Initialize variables - simple count in nibbles:
			h0 = 0x67452301;
			h1 = 0xefcdab89;
			h2 = 0x98badcfe;
			h3 = 0x10325476;

			for (new_len = initial_len + 1; new_len % (512 / 8) != 448 / 8; new_len++)
				;

#ifdef _KERNEL_MODE
			msg = (UINT8*)ExAllocatePoolWithTag(PagedPool, new_len + 8, POOLTAG);
#else
			msg = (UINT8*)malloc((size_t)new_len + 8);
#endif
			if (!msg)
			{
				return;
			}
			memcpy(msg, initial_msg, initial_len);
			msg[initial_len] = 0x80; // append the "1" bit; most significant bit is "first"
			for (offset = initial_len + 1; offset < new_len; offset++)
				msg[offset] = 0; // append "0" bits

			// append the len in bits at the end of the buffer.
			to_bytes(initial_len * 8, msg + new_len);
			// initial_len>>29 == initial_len*8>>32, but avoids overflow.
			to_bytes(initial_len >> 29, msg + new_len + 4);

			// Process the message in successive 512-bit chunks:
			//for each 512-bit chunk of message:
			for (offset = 0; offset < new_len; offset += (512 / 8)) {
				for (i = 0; i < 16; i++)
					w[i] = to_int32(msg + offset + i * 4);

				// Initialize hash value for this chunk:
				a = h0;
				b = h1;
				c = h2;
				d = h3;

				// Main loop:
				for (i = 0; i < 64; i++) {

					if (i < 16) {
						f = (b & c) | ((~b) & d);
						g = i;
					}
					else if (i < 32) {
						f = (d & b) | ((~d) & c);
						g = (5 * i + 1) % 16;
					}
					else if (i < 48) {
						f = b ^ c ^ d;
						g = (3 * i + 5) % 16;
					}
					else {
						f = c ^ (b | (~d));
						g = (7 * i) % 16;
					}

					temp = d;
					d = c;
					c = b;
					b = b + LEFTROTATE((a + f + k[i] + w[g]), r[i]);
					a = temp;

				}

				// Add this chunk's hash to result so far:
				h0 += a;
				h1 += b;
				h2 += c;
				h3 += d;

			}

			// cleanup
#ifdef _KERNEL_MODE
			ExFreePoolWithTag(msg, POOLTAG);
#else
			free(msg);
#endif


			//var char digest[16] := h0 append h1 append h2 append h3 //(Output is in little-endian)
			to_bytes(h0, digest);
			to_bytes(h1, digest + 4);
			to_bytes(h2, digest + 8);
			to_bytes(h3, digest + 12);
		}


		inline std::string Md5(PVOID buffer, ULONG bufferLen)
		{
			UINT8 raw[16];
			std::string res;

			md5((const UINT8*)buffer, bufferLen, raw);

			for (size_t i = 0; i < 16; i++)
			{
				char ch[4] = { 0 };
				snprintf(ch, ARRAYSIZE(ch), "%02x", raw[i]);
				res += ch;
			}

			return res;
		}

		inline std::string Md5(std::string filePath)
		{
			std::ifstream file(filePath, std::ios::binary | std::ios::ate);
			std::streamsize fileSize = file.tellg();
			file.seekg(0, std::ios::beg);

			std::vector<char> fileData(fileSize);
			if (file.read(fileData.data(), fileSize))
			{
				std::string pdbMd5 = md5::Md5(fileData.data(), (ULONG)fileSize);
				return pdbMd5;
			}
			return "";
		}

	}
#pragma warning(pop)

	// 动态启用TLS 1.2支持的辅助函数
	inline BOOL EnableTLS12Support() {
		HKEY hKey;
		DWORD dwValue;
		//DWORD dwSize = sizeof(DWORD);
		LONG result;
		
		// 1. 启用TLS 1.2客户端支持
		result = RegCreateKeyExA(HKEY_LOCAL_MACHINE,
			"SYSTEM\\CurrentControlSet\\Control\\SecurityProviders\\SCHANNEL\\Protocols\\TLS 1.2\\Client",
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
		
		if (result == ERROR_SUCCESS) {
			// 设置 DisabledByDefault = 0
			dwValue = 0;
			RegSetValueExA(hKey, "DisabledByDefault", 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue));
			
			// 设置 Enabled = 1
			dwValue = 1;
			RegSetValueExA(hKey, "Enabled", 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue));
			
			RegCloseKey(hKey);
			printf("ezpdb TLS 1.2客户端支持已启用\n");
		} else {
			printf("ezpdb 警告：无法修改TLS 1.2注册表设置 (错误码: %ld)\n", result);
		}

        
		// 2. 对于.NET应用，启用强加密
		result = RegCreateKeyExA(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\.NETFramework\\v4.0.30319",
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
		
		if (result == ERROR_SUCCESS) {
			dwValue = 1;
			RegSetValueExA(hKey, "SchUseStrongCrypto", 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue));
			RegCloseKey(hKey);
			
		}
        /*
		// 3. 对于32位应用在64位系统上
		result = RegCreateKeyExA(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Wow6432Node\\Microsoft\\.NETFramework\\v4.0.30319",
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
		
		if (result == ERROR_SUCCESS) {
			dwValue = 1;
			RegSetValueExA(hKey, "SchUseStrongCrypto", 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue));
			RegCloseKey(hKey);
		}
		*/

		return TRUE;
	}

	// 使用WinHTTP下载的现代方法 (支持TLS 1.2)
	inline BOOL DownloadFileWithWinHTTP(const std::string& url, const std::string& filePath) {
		printf("ezpdb 尝试使用WinHTTP下载...\n");
		
		// 解析URL
		std::string hostname, path;
		bool isHttps = false;
		
		if (url.find("https://") == 0) {
			isHttps = true;
			size_t start = 8; // "https://"的长度
			size_t pathStart = url.find('/', start);
			if (pathStart != std::string::npos) {
				hostname = url.substr(start, pathStart - start);
				path = url.substr(pathStart);
			} else {
				hostname = url.substr(start);
				path = "/";
			}
		} else if (url.find("http://") == 0) {
			size_t start = 7; // "http://"的长度
			size_t pathStart = url.find('/', start);
			if (pathStart != std::string::npos) {
				hostname = url.substr(start, pathStart - start);
				path = url.substr(pathStart);
			} else {
				hostname = url.substr(start);
				path = "/";
			}
		} else {
			return FALSE;
		}
		
		// 转换为宽字符
		int hostnameLen = MultiByteToWideChar(CP_UTF8, 0, hostname.c_str(), -1, NULL, 0);
		int pathLen = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, NULL, 0);
		
		std::vector<wchar_t> wHostname(hostnameLen);
		std::vector<wchar_t> wPath(pathLen);
		
		MultiByteToWideChar(CP_UTF8, 0, hostname.c_str(), -1, wHostname.data(), hostnameLen);
		MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, wPath.data(), pathLen);
		
		// 打开WinHTTP会话
		HINTERNET hSession = WinHttpOpen(L"ezpdb/1.0", 
										 WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
										 WINHTTP_NO_PROXY_NAME, 
										 WINHTTP_NO_PROXY_BYPASS, 
										 0);
		if (!hSession) {
			printf("ezpdb WinHttpOpen失败\n");
			return FALSE;
		}
		
		// 连接到服务器
		INTERNET_PORT port = isHttps ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
		HINTERNET hConnect = WinHttpConnect(hSession, wHostname.data(), port, 0);
		if (!hConnect) {
			printf("ezpdb WinHttpConnect失败\n");
			WinHttpCloseHandle(hSession);
			return FALSE;
		}
		
		// 创建请求
		DWORD dwFlags = isHttps ? WINHTTP_FLAG_SECURE : 0;
		HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", wPath.data(),
												NULL, WINHTTP_NO_REFERER,
												WINHTTP_DEFAULT_ACCEPT_TYPES,
												dwFlags);
		if (!hRequest) {
			printf("ezpdb WinHttpOpenRequest失败\n");
			WinHttpCloseHandle(hConnect);
			WinHttpCloseHandle(hSession);
			return FALSE;
		}
		
		// 设置TLS 1.2
		if (isHttps) {
			DWORD dwTlsFlags = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
			WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURE_PROTOCOLS, &dwTlsFlags, sizeof(dwTlsFlags));
			
			// 忽略证书错误 (用于测试)
			DWORD dwSecurityFlags = SECURITY_FLAG_IGNORE_CERT_CN_INVALID | 
								   SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
								   SECURITY_FLAG_IGNORE_UNKNOWN_CA;
			WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwSecurityFlags, sizeof(dwSecurityFlags));
		}
		
		// 发送请求
		BOOL bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
										   WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
		if (!bResults) {
			printf("ezpdb WinHttpSendRequest失败，错误: %lu\n", GetLastError());
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			WinHttpCloseHandle(hSession);
			return FALSE;
		}
		
		// 接收响应
		bResults = WinHttpReceiveResponse(hRequest, NULL);
		if (!bResults) {
			printf("ezpdb WinHttpReceiveResponse失败\n");
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			WinHttpCloseHandle(hSession);
			return FALSE;
		}
		
		// 创建输出文件
		HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			printf("ezpdb 无法创建文件: %s\n", filePath.c_str());
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			WinHttpCloseHandle(hSession);
			return FALSE;
		}
		
		// 读取数据并写入文件
		DWORD dwSize = 0;
		DWORD dwDownloaded = 0;
		DWORD dwWritten = 0;
		BYTE buffer[4096];
		BOOL success = TRUE;
		
		do {
			// 检查可用数据
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
				printf("ezpdb WinHttpQueryDataAvailable失败\n");
				success = FALSE;
				break;
			}
			
			if (dwSize == 0) break;
			
			// 读取数据
			dwSize = min(dwSize, sizeof(buffer));
			if (!WinHttpReadData(hRequest, buffer, dwSize, &dwDownloaded)) {
				printf("ezpdb WinHttpReadData失败\n");
				success = FALSE;
				break;
			}
			
			// 写入文件
			if (!WriteFile(hFile, buffer, dwDownloaded, &dwWritten, NULL) || dwWritten != dwDownloaded) {
				printf("ezpdb 文件写入失败\n");
				success = FALSE;
				break;
			}
			
		} while (dwSize > 0);
		
		// 清理资源
		CloseHandle(hFile);
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		
		if (success) {
			printf("ezpdb WinHTTP下载成功!\n");
		} else {
			DeleteFileA(filePath.c_str()); // 删除不完整的文件
		}
		
		return success;
	}

	class pdb
	{
	private:
		struct PDBHeader7
		{
			char signature[0x20];
			int page_size;
			int allocation_table_pointer;
			int file_page_count;
			int root_stream_size;
			int reserved;
			int root_stream_page_number_list_number;
		};

		struct RootStream7
		{
			int num_streams;
			int stream_sizes[1]; //num_streams
		};

		struct GUID_StreamData
		{
			int ver;
			int date;
			int age;
			GUID guid;
		};

		struct PdbInfo
		{
			DWORD	Signature;
			GUID	Guid;
			DWORD	Age;
			char	PdbFileName[1];
		};

#define EZ_PDB_BASE_OF_DLL (DWORD64)0x10000000

		std::string _pe_path;
		std::string _local_pdb_path;
		std::string _current_pdb_path;  // 新增：存储当前使用的PDB路径
		bool pdb_downloaded;
		bool pdb_loaded;
		HANDLE _hPdbFile;
		HANDLE _hProcess;
		std::string _symbol_server;

		std::string download(std::string pe_path, bool bRedownload = false)
		{
			// download pdb file from symbol server
			// return pdb path if success, 
			// or return empty string if failed, user can call GetLastError() to know wth is going on
			std::string pdbDownloadPath;

			WCHAR wszCurrentDir[MAX_PATH] = { 0 };
			GetModuleFileNameW(NULL, wszCurrentDir, _countof(wszCurrentDir));
			
			// 使用更兼容的方法移除文件名，兼容Windows 7
			WCHAR* lastSlash = wcsrchr(wszCurrentDir, L'\\');
			if (lastSlash != NULL) {
				*lastSlash = L'\0';  // 截断到最后一个反斜杠
			}
			
			std::wstring wsCurrentDir = wszCurrentDir;
			
			//setup converter
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

			//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
			std::string CurrentDir = converter.to_bytes(wsCurrentDir);
			
			// 调试输出：打印当前目录
			printf("ezpdb CurrentDir: %s\n", CurrentDir.c_str());
			
			// 验证路径是否有效（不应该包含.exe）
			if (CurrentDir.find(".exe") != std::string::npos) {
				printf("ezpdb Warning: CurrentDir contains .exe, attempting fallback\n");
				// 备用方案：使用临时目录
				char tempPath[MAX_PATH];
				GetTempPathA(sizeof(tempPath), tempPath);
				CurrentDir = tempPath;
				printf("ezpdb Fallback CurrentDir: %s\n", CurrentDir.c_str());
			}
			
			if (CurrentDir.back() != '\\')
			{
				CurrentDir += '\\';
			}
			pdbDownloadPath = CurrentDir += "symbols\\";

			// make sure the directory exist
			if (!CreateDirectoryA(pdbDownloadPath.c_str(), NULL))
			{
				if (GetLastError() != ERROR_ALREADY_EXISTS)
				{
					printf("ezpdb CreateDirectory failed, error: %d\n", GetLastError());
					printf("ezpdb Attempted path: %s\n", pdbDownloadPath.c_str());
					return "";
				}
			}

			// read pe file

#ifndef _AMD64_
			PVOID OldValue = NULL;
			Wow64DisableWow64FsRedirection(&OldValue);
#endif

			std::ifstream file(pe_path, std::ios::binary | std::ios::ate);
			std::streamsize size = file.tellg();
			file.seekg(0, std::ios::beg);
			std::vector<char> buffer(size);

#ifndef _AMD64_
			Wow64RevertWow64FsRedirection(&OldValue);
#endif

			if (!file.read(buffer.data(), size) || size == 0)
			{
				printf("ezpdb read file failed: %s\n", pe_path.c_str());
				SetLastError(ERROR_ACCESS_DENIED);
				return "";
			}

			//std::string pdbPath = pdbDownloadPath + "ntkrnlmp" + ".pdb";

			std::string pdbPath = pdbDownloadPath + md5::Md5(buffer.data(), (ULONG)size) + ".pdb";
			//std::string pdbPath = "C:\\Windows\\Temp\\ntkrnlmp.pdb";
			printf("ezpdb pdbDownloadPath: %s\n", pdbDownloadPath.c_str());
            printf("ezpdb pdbPath: %s\n", pdbPath.c_str());
			std::string pdbHashCachePath = pdbPath + ".md5";

			// 设置当前PDB路径
			_current_pdb_path = pdbPath;

			if (!bRedownload)
			{
				if (PathFileExistsA(pdbPath.c_str()))
				{
					printf("ezpdb PDB file exists: %s\n", pdbPath.c_str());
					if (PathFileExistsA(pdbHashCachePath.c_str()))
					{
						std::ifstream ifs(pdbHashCachePath);
						std::string md5Cache;
						ifs >> md5Cache;
						ifs.close();

						std::string pdbMd5 = md5::Md5(pdbPath);

						if (pdbMd5 == md5Cache)
						{
							printf("ezpdb PDB hash matched, returning existing file\n");
							return pdbPath;
						}
					}
				}
			}

			// get pdb info from debug info directory
			IMAGE_DOS_HEADER* pDos = (IMAGE_DOS_HEADER*)buffer.data();
			IMAGE_NT_HEADERS* pNT = (IMAGE_NT_HEADERS*)(buffer.data() + pDos->e_lfanew);
			IMAGE_FILE_HEADER* pFile = &pNT->FileHeader;
			IMAGE_OPTIONAL_HEADER64* pOpt64 = NULL;
			IMAGE_OPTIONAL_HEADER32* pOpt32 = NULL;
			BOOL x86 = FALSE;
			if (pFile->Machine == IMAGE_FILE_MACHINE_AMD64)
			{
				pOpt64 = (IMAGE_OPTIONAL_HEADER64*)(&pNT->OptionalHeader);
			}
			else if (pFile->Machine == IMAGE_FILE_MACHINE_I386)
			{
				pOpt32 = (IMAGE_OPTIONAL_HEADER32*)(&pNT->OptionalHeader);
				x86 = TRUE;
			}
			else
			{
				SetLastError(ERROR_NOT_SUPPORTED);
				return "";
			}
			DWORD ImageSize = x86 ? pOpt32->SizeOfImage : pOpt64->SizeOfImage;

			// file buffer to image buffer
			PBYTE ImageBuffer = (PBYTE)malloc(ImageSize);
			if (!ImageBuffer)
			{
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				return "";
			}
			memcpy(ImageBuffer, buffer.data(), x86 ? pOpt32->SizeOfHeaders : pOpt64->SizeOfHeaders);
			IMAGE_SECTION_HEADER* pCurrentSectionHeader = IMAGE_FIRST_SECTION(pNT);
			for (UINT i = 0; i != pFile->NumberOfSections; ++i, ++pCurrentSectionHeader)
			{
				if (pCurrentSectionHeader->SizeOfRawData)
				{
					memcpy(ImageBuffer + pCurrentSectionHeader->VirtualAddress, buffer.data() + pCurrentSectionHeader->PointerToRawData, pCurrentSectionHeader->SizeOfRawData);
				}
			}
			IMAGE_DATA_DIRECTORY* pDataDir = nullptr;
			if (x86)
			{
				pDataDir = &pOpt32->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
			}
			else
			{
				pDataDir = &pOpt64->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
			}
			IMAGE_DEBUG_DIRECTORY* pDebugDir = (IMAGE_DEBUG_DIRECTORY*)(ImageBuffer + pDataDir->VirtualAddress);
			if (!pDataDir->Size || IMAGE_DEBUG_TYPE_CODEVIEW != pDebugDir->Type)
			{
				// invalid debug dir
				free(ImageBuffer);
				SetLastError(ERROR_NOT_SUPPORTED);
				return "";
			}
			PdbInfo* pdb_info = (PdbInfo*)(ImageBuffer + pDebugDir->AddressOfRawData);
			if (pdb_info->Signature != 0x53445352)
			{
				// invalid debug dir
				free(ImageBuffer);
				SetLastError(ERROR_NOT_SUPPORTED);
				return "";
			}

			// download pdb
            /*
            ImGui::OpenPopup("download");
            if (ImGui::BeginPopupModal("download", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("can't find pdb,start download");
                ImGui::Separator();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                ImGui::EndPopup();
            }
            */
            MessageBox(0,"click to download pdb",NULL,MB_OK);
			DeleteFileA(pdbPath.c_str());
			wchar_t w_GUID[100] = { 0 };
			if (!StringFromGUID2(pdb_info->Guid, w_GUID, 100))
			{
				free(ImageBuffer);
				SetLastError(ERROR_NOT_SUPPORTED);
				return "";
			}
			char a_GUID[100]{ 0 };
			size_t l_GUID = 0;
			if (wcstombs_s(&l_GUID, a_GUID, w_GUID, sizeof(a_GUID)) || !l_GUID)
			{
				free(ImageBuffer);
				SetLastError(ERROR_NOT_SUPPORTED);
				return "";
			}

			char guid_filtered[256] = { 0 };
			for (UINT i = 0; i != l_GUID; ++i)
			{
				if ((a_GUID[i] >= '0' && a_GUID[i] <= '9') || (a_GUID[i] >= 'A' && a_GUID[i] <= 'F') || (a_GUID[i] >= 'a' && a_GUID[i] <= 'f'))
				{
					guid_filtered[strlen(guid_filtered)] = a_GUID[i];
				}
			}

			char age[3] = { 0 };
			_itoa_s(pdb_info->Age, age, 10);

			// 备用符号服务器列表
			std::vector<std::string> symbol_servers = {
				_symbol_server,  // 用户指定的服务器
				"https://msdl.microsoft.com/download/symbols/",  // 微软官方
				"https://symbols.mozilla.org/"  // Mozilla备用
			};

			std::string url;
			HRESULT hr = E_FAIL;
			
			// 动态启用TLS 1.2支持
			EnableTLS12Support();
			
			// Windows 7 TLS兼容性设置
			#ifndef WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2
			#define WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2 0x00000800
			#endif
			
			// 尝试启用TLS 1.2支持
			HINTERNET hSession = InternetOpenA("ezpdb/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
			if (hSession) {
				DWORD dwFlags = SECURITY_FLAG_IGNORE_CERT_CN_INVALID | 
							   SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
							   SECURITY_FLAG_IGNORE_UNKNOWN_CA;
				InternetSetOptionA(hSession, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
				InternetCloseHandle(hSession);
			}

			// 尝试每个符号服务器
			for (int attempt = 0; attempt < symbol_servers.size(); attempt++) {
				url = symbol_servers[attempt];
				url += pdb_info->PdbFileName;
				url += "/";
				url += guid_filtered;
				url += age;
				url += "/";
				url += pdb_info->PdbFileName;

				printf("ezpdb Download URL (attempt %d): %s\n", attempt + 1, url.c_str());
				printf("ezpdb Target Path: %s\n", pdbPath.c_str());

				// 首先尝试URLDownloadToFileA
				printf("ezpdb Starting download from server %d (URLDownloadToFileA)...\n", attempt + 1);
				hr = URLDownloadToFileA(NULL, url.c_str(), pdbPath.c_str(), NULL, NULL);
				
				if (SUCCEEDED(hr)) {
					printf("ezpdb Download successful from server %d!\n", attempt + 1);
					break;
				} else {
					printf("ezpdb URLDownloadToFileA failed from server %d (HRESULT: 0x%08X)\n", attempt + 1, hr);
					
					// 如果URLDownloadToFileA失败，尝试WinHTTP
					printf("ezpdb Trying WinHTTP for server %d...\n", attempt + 1);
					if (DownloadFileWithWinHTTP(url, pdbPath)) {
						hr = S_OK;
						printf("ezpdb WinHTTP download successful from server %d!\n", attempt + 1);
						break;
					} else {
						printf("ezpdb WinHTTP also failed for server %d\n", attempt + 1);
					}
				}
			}
			
			if (FAILED(hr))
			{
				printf("ezpdb URLDownloadToFileA failed with HRESULT: 0x%08X\n", hr);
				
				// 提供具体的错误信息
				switch (hr) {
					case E_OUTOFMEMORY:
						printf("ezpdb Error: Out of memory\n");
						break;
					case INET_E_DOWNLOAD_FAILURE:
						printf("ezpdb Error: Download failure - check network connection\n");
						break;
					case INET_E_CANNOT_CONNECT:
						printf("ezpdb Error: Cannot connect to server\n");
						break;
					case INET_E_RESOURCE_NOT_FOUND:
						printf("ezpdb Error: PDB file not found on server\n");
						break;
					case INET_E_SECURITY_PROBLEM:
						printf("ezpdb Error: Security/TLS problem - try manual download\n");
						break;
					default:
						printf("ezpdb Error: Unknown error 0x%08X\n", hr);
						break;
				}
				
				printf("ezpdb Manual download instructions:\n");
				printf("ezpdb 1. Open browser and go to: %s\n", url.c_str());
				printf("ezpdb 2. Save the file as: %s\n", pdbPath.c_str());
				printf("ezpdb 3. 寄了，下不了\n");
				
				free(ImageBuffer);
				return "";
			}
			
			// 验证下载的文件
			if (GetFileAttributesA(pdbPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
				printf("ezpdb Error: Downloaded file not found or invalid\n");
				free(ImageBuffer);
				return "";
			}
			
			printf("ezpdb Download completed successfully!\n");

			free(ImageBuffer);

			std::string pdbMd5 = md5::Md5(pdbPath);
			std::ofstream ofs(pdbHashCachePath);
			ofs << pdbMd5;

			return pdbPath;
		}

	public:
		// constructor for downloading pdb from symbol server
		pdb(std::string pe_path, std::string symbol_server = "https://msdl.microsoft.com/download/symbols/")
		{
			_pe_path = pe_path;
			_symbol_server = symbol_server;
			pdb_downloaded = false;
			pdb_loaded = false;
		}

		// constructor for local pdb file
		explicit pdb(std::string pdb_path)
		{
			_pe_path = "";
			_symbol_server = "";
			_local_pdb_path = pdb_path;
			pdb_downloaded = false;
			pdb_loaded = false;
		}

		bool init()
		{
			std::string pdb_path;
			
			if (!_local_pdb_path.empty())
			{
				// use local pdb file
				pdb_path = _local_pdb_path;
			}
			else
			{
				// download pdb from symbol server
				pdb_path = download(_pe_path);
				if (pdb_path.empty())
				{
					return false;
				}
				pdb_downloaded = true;
			}
			
			// 设置当前PDB路径
			_current_pdb_path = pdb_path;

			// get pdb file size
			WIN32_FILE_ATTRIBUTE_DATA file_attr_data{ 0 };
			if (!GetFileAttributesExA(pdb_path.c_str(), GetFileExInfoStandard, &file_attr_data))
			{
				return false;
			}
			auto pdbSize = file_attr_data.nFileSizeLow;

			// open pdb file
			HANDLE hPdbFile = CreateFileA(pdb_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
			if (hPdbFile == INVALID_HANDLE_VALUE)
			{
				return false;
			}

			// open current process
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, GetCurrentProcessId());
			if (!hProcess)
			{
				CloseHandle(hPdbFile);
				return false;
			}

			// Initializes the symbol handler for a process
			if (!SymInitialize(hProcess, pdb_path.c_str(), FALSE))
			{
				CloseHandle(hProcess);
				CloseHandle(hPdbFile);
				return false;
			}

			SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_AUTO_PUBLICS | SYMOPT_DEBUG | SYMOPT_LOAD_ANYTHING);

			DWORD64 SymbolTable = SymLoadModuleEx(hProcess, NULL, pdb_path.c_str(), NULL, EZ_PDB_BASE_OF_DLL, pdbSize, NULL, NULL);
			if (!SymbolTable)
			{
				SymCleanup(hProcess);
				CloseHandle(hProcess);
				CloseHandle(hPdbFile);
				return false;
			}

			_hPdbFile = hPdbFile;
			_hProcess = hProcess;

			pdb_loaded = true;

			return true;
		}

		// init with local pdb file path
		bool init(std::string pdb_path)
		{
			// check if pdb file exists
			if (!PathFileExistsA(pdb_path.c_str()))
			{
				SetLastError(ERROR_FILE_NOT_FOUND);
				return false;
			}

			// get pdb file size
			WIN32_FILE_ATTRIBUTE_DATA file_attr_data{ 0 };
			if (!GetFileAttributesExA(pdb_path.c_str(), GetFileExInfoStandard, &file_attr_data))
			{
				return false;
			}
			auto pdbSize = file_attr_data.nFileSizeLow;

			// open pdb file
			HANDLE hPdbFile = CreateFileA(pdb_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
			if (hPdbFile == INVALID_HANDLE_VALUE)
			{
				return false;
			}

			// open current process
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, GetCurrentProcessId());
			if (!hProcess)
			{
				CloseHandle(hPdbFile);
				return false;
			}

			// initialize symbol handler
			if (!SymInitialize(hProcess, NULL, FALSE))
			{
				CloseHandle(hProcess);
				CloseHandle(hPdbFile);
				return false;
			}

			SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_AUTO_PUBLICS | SYMOPT_DEBUG | SYMOPT_LOAD_ANYTHING);

			DWORD64 SymbolTable = SymLoadModuleEx(hProcess, NULL, pdb_path.c_str(), NULL, EZ_PDB_BASE_OF_DLL, pdbSize, NULL, NULL);
			if (!SymbolTable)
			{
				SymCleanup(hProcess);
				CloseHandle(hProcess);
				CloseHandle(hPdbFile);
				return false;
			}

			_hPdbFile = hPdbFile;
			_hProcess = hProcess;
			pdb_loaded = true;

			return true;
		}

		~pdb()
		{
			if (pdb_loaded)
			{
				SymUnloadModule64(_hProcess, EZ_PDB_BASE_OF_DLL);
				SymCleanup(_hProcess);
				CloseHandle(_hProcess);
				CloseHandle(_hPdbFile);
			}
		}

		// get function / global variable rva. return -1 if failed
		int get_rva(std::string SymName)
		{
			SYMBOL_INFO si = { 0 };
			si.SizeOfStruct = sizeof(SYMBOL_INFO);
			if (!SymFromName(_hProcess, SymName.c_str(), &si))
			{
				return -1;
			}
			return (int)(si.Address - si.ModBase);
		}

		// get struct attribute offset. return -1 if failed
		int get_attribute_offset(std::string StructName, std::wstring PropertyName)
		{
			ULONG SymInfoSize = sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR);
			SYMBOL_INFO* SymInfo = (SYMBOL_INFO*)malloc(SymInfoSize);
			if (!SymInfo)
			{
				return -1;
			}
			ZeroMemory(SymInfo, SymInfoSize);
			SymInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
			SymInfo->MaxNameLen = MAX_SYM_NAME;
			if (!SymGetTypeFromName(_hProcess, EZ_PDB_BASE_OF_DLL, StructName.c_str(), SymInfo))
			{
				return -1;
			}

			TI_FINDCHILDREN_PARAMS TempFp = { 0 };
			if (!SymGetTypeInfo(_hProcess, EZ_PDB_BASE_OF_DLL, SymInfo->TypeIndex, TI_GET_CHILDRENCOUNT, &TempFp))
			{
				free(SymInfo);
				return -1;
			}

			ULONG ChildParamsSize = sizeof(TI_FINDCHILDREN_PARAMS) + TempFp.Count * sizeof(ULONG);
			TI_FINDCHILDREN_PARAMS* ChildParams = (TI_FINDCHILDREN_PARAMS*)malloc(ChildParamsSize);
			if (ChildParams == NULL)
			{
				free(SymInfo);
				return -1;
			}
			ZeroMemory(ChildParams, ChildParamsSize);
			memcpy(ChildParams, &TempFp, sizeof(TI_FINDCHILDREN_PARAMS));
			if (!SymGetTypeInfo(_hProcess, EZ_PDB_BASE_OF_DLL, SymInfo->TypeIndex, TI_FINDCHILDREN, ChildParams))
			{
				goto failed;
			}
			for (ULONG i = ChildParams->Start; i < ChildParams->Count; i++)
			{
				WCHAR* pSymName = NULL;
				ULONG Offset = 0;
				if (!SymGetTypeInfo(_hProcess, EZ_PDB_BASE_OF_DLL, ChildParams->ChildId[i], TI_GET_OFFSET, &Offset))
				{
					goto failed;
				}
				if (!SymGetTypeInfo(_hProcess, EZ_PDB_BASE_OF_DLL, ChildParams->ChildId[i], TI_GET_SYMNAME, &pSymName))
				{
					goto failed;
				}
				if (pSymName)
				{
					if (wcscmp(pSymName, PropertyName.c_str()) == 0)
					{
						LocalFree(pSymName);
						free(ChildParams);
						free(SymInfo);
						return int(Offset);
					}
				}
			}
		failed:
			free(ChildParams);
			free(SymInfo);
			return -1;
		}

		// get struct size, failed return -1
		int get_struct_size(std::string StructName)
		{
			ULONG SymInfoSize = sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR);
			SYMBOL_INFO* SymInfo = (SYMBOL_INFO*)malloc(SymInfoSize);
			if (!SymInfo)
			{
				return -1;
			}
			ZeroMemory(SymInfo, SymInfoSize);
			SymInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
			SymInfo->MaxNameLen = MAX_SYM_NAME;
			if (!SymGetTypeFromName(_hProcess, EZ_PDB_BASE_OF_DLL, StructName.c_str(), SymInfo))
			{
				return -1;
			}
			return int(SymInfo->Size);
		}

		// get function name from rva. return empty string if failed
		std::string get_function_name(int rva)
		{
			DWORD64 address = EZ_PDB_BASE_OF_DLL + rva;
			DWORD64 displacement = 0;
			
			char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
			SYMBOL_INFO* symbol = (SYMBOL_INFO*)buffer;
			symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
			symbol->MaxNameLen = MAX_SYM_NAME;
			
			if (SymFromAddr(_hProcess, address, &displacement, symbol))
			{
				return std::string(symbol->Name);
			}
			return "";
		}

		// get function name from va and module base. return empty string if failed
		std::string get_function_name_from_base_32(DWORD va, DWORD module_base)
		{
			int rva = va - module_base;  // 自动计算RVA
			DWORD64 address = EZ_PDB_BASE_OF_DLL + rva;
			DWORD64 displacement = 0;

			char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
			SYMBOL_INFO* symbol = (SYMBOL_INFO*)buffer;
			symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
			symbol->MaxNameLen = MAX_SYM_NAME;

			if (SymFromAddr(_hProcess, address, &displacement, symbol))
			{
				return std::string(symbol->Name);
			}
			return "";
		}

		// 获取当前PDB文件的路径
		std::string get_current_pdb_path() const
		{
			return _current_pdb_path;
		}

	};
}








