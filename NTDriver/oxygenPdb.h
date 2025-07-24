#pragma once
#include <fltKernel.h>

namespace oxygenPdb {

	class Pdber {

	public:
		Pdber(const wchar_t* moduleName);
        ~Pdber();
		bool init();
        void setPdbDownloadPath(const UNICODE_STRING& downloadPath);
        void setPdbDownloadPath(const wchar_t* downloadPath);

		ULONG_PTR GetPointer(const char* name);
		size_t GetOffset(const char* structName, const char* propertyName);

	private:
		char padding[2000];//can not revise this!!! else it will ocurrs stack overflow!!
	};

}
