#pragma once
/*
Moduler is a product class in the Factory Pattern,
and in combination with the PDB Viewer class,
it can parse the symbols within the module.
*/
#include <fltKernel.h>
#include <kstl/memory.h>
#include "viewer.hpp"
#include <kstl/string.h>
#include "moduler.hpp"
#include "./air14/FileStream.h"
#include "./air14/MsfReader.h"
#include "./air14/SymbolExtractor.h"
#include "./air14/StructExtractor.h"
namespace oxygenPdb {


	class Pdber {

	public:
		Pdber(const wchar_t* moduleName);
		~Pdber();
		bool init();
		ULONG_PTR GetPointer(const char* name);
		size_t GetOffset(const char* structName, const char* propertyName);
		void GetName(ULONG_PTR address, UNICODE_STRING* outStr);

        PdbViewer _pdbViewer;//Responsible for downloading and parsing PDB.
	private:
		using ptr_t = UINT_PTR;
		
		Moduler _moduler;//responsible for get module base..
		char _pdbGuid[100];
		//kstd::unique_ptr<unsigned char[]> _pdbBuf;
		wchar_t _pdbPath[MAX_PATH];
		symbolic_access::SymbolsMap _symbols;
		symbolic_access::StructsMap _structs;
		bool _initfailed;
	};



	ULONG_PTR Pdber::GetPointer(const char* name)
	{
		if (_initfailed) return 0;
		std::string_view SymbolName(name);

		const auto& iter = _symbols.find(SymbolName);
		if (iter == _symbols.end())
			return 0;

		return (ULONG_PTR)(_moduler.getModuleBase() + iter->second);
	}

	//Get offset
	size_t Pdber::GetOffset(const char* structName, const char* propertyName) {

		const auto& structsIter = this->_structs.find(structName);
		if (structsIter == _structs.end())
			return 0;

		const auto& membersIter = std::find_if(structsIter->second.begin(), structsIter->second.end(),
			[&](const auto& MemberNameAndOffset) { return MemberNameAndOffset.Name == propertyName; });

		if (membersIter == structsIter->second.end())
			return 0;

		return membersIter->Offset;
	}

	void Pdber::GetName(ULONG_PTR address, UNICODE_STRING* outStr)
	{
        {
            if (_initfailed || !outStr) {
                if (outStr) {
                    outStr->Length = 0;
                    outStr->MaximumLength = 0;
                    outStr->Buffer = nullptr;
                }
                return;
            }

            auto base = _moduler.getModuleBase();
            ULONG_PTR rva = address - base;

            const char* result = nullptr;
            ULONG_PTR bestRva = 0;
            for (const auto& [name, symRva] : _symbols) {
                if (symRva <= rva && symRva >= bestRva) {
                    bestRva = symRva;
                    result = name.c_str();
                }
            }

            if (result) {
                // 1. 计算长度
                size_t len = strlen(result);
                size_t wlen = len * sizeof(wchar_t);

                // 2. 分配UNICODE_STRING缓冲区
                outStr->Length = (USHORT)wlen;
                outStr->MaximumLength = (USHORT)(wlen + sizeof(wchar_t));
                outStr->Buffer = (PWCH)ExAllocatePoolWithTag(NonPagedPool, outStr->MaximumLength, 'symN');
                if (!outStr->Buffer) {
                    outStr->Length = 0;
                    outStr->MaximumLength = 0;
                    return;
                }

                // 3. 转换为宽字符
                for (size_t i = 0; i < len; ++i) {
                    outStr->Buffer[i] = (wchar_t)result[i];
                }
                outStr->Buffer[len] = L'\0';
            }
            else {
                outStr->Length = 0;
                outStr->MaximumLength = 0;
                outStr->Buffer = nullptr;
            }
        }
	}

}
