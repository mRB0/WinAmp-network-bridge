#include "stdafx.h"
#include "filelist.h"

#include <memory>

#include "util.h"

static std::shared_ptr<HANDLE> wrapFileHandle(HANDLE rawHandle) {
	return std::shared_ptr<HANDLE>(
		new HANDLE{ rawHandle },
		[](HANDLE *ptr)
		{
			FindClose(*ptr);
			delete ptr;
		}
	);
}

std::list<std::wstring> listFiles(std::wstring const &directory, std::wstring const &fileSpec) {
	std::wstring const searchPath = directory + L"\\" + fileSpec;

	WIN32_FIND_DATAW findResult;

	auto fileList = std::list<std::wstring>();

	HANDLE rawHandle = FindFirstFileW(searchPath.c_str(), &findResult);

	if (rawHandle == INVALID_HANDLE_VALUE) {
		DWORD lastError = GetLastError();
		if (lastError == ERROR_FILE_NOT_FOUND) {
			return fileList;
		}
		else {
			throw std::ios_base::failure(logError(lastError));
		}
	}

	auto handle = wrapFileHandle(rawHandle);

	do {
		fileList.push_back(directory + L"\\" + std::wstring(findResult.cFileName));
	} while (FindNextFileW(*handle, &findResult));

	DWORD lastError = GetLastError();
	if (lastError != ERROR_NO_MORE_FILES) {
		throw std::ios_base::failure(logError(lastError));
	}

	return fileList;
}
