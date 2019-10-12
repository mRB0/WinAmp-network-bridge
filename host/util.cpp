#include "stdafx.h"
#include "util.h"

#include <memory>

#include "wxx_wincore.h"

std::wstring getErrorDescription(DWORD error) {
	CStringW str;
	str.GetErrorString(error);

	return std::wstring(str.c_str());
}

std::exception logError(DWORD error) {
	auto errorDescription = getErrorDescription(error);
	odslog(L"Error: " << errorDescription << std::endl);

	return std::exception(wstringToMultiByte(errorDescription, CP_ACP).c_str());
}

std::list<std::wstring> listFiles(std::wstring const &directory, std::wstring const &fileSpec) {
	std::wstring const searchPath = directory + L"\\" + fileSpec;

	WIN32_FIND_DATAW findResult;

	auto fileList = std::list<std::wstring>();

	HANDLE handle = FindFirstFileW(searchPath.c_str(), &findResult);

	if (handle == INVALID_HANDLE_VALUE) {
		DWORD lastError = GetLastError();
		if (lastError == ERROR_FILE_NOT_FOUND) {
			return fileList;
		}
		else {
			throw logError(lastError);
		}
	}

	auto pHandle = std::shared_ptr<HANDLE>(&handle, [](HANDLE *ptr) { FindClose(*ptr); });

	do {
		fileList.push_back(directory + L"\\" + std::wstring(findResult.cFileName));
	} while (FindNextFileW(handle, &findResult));

	DWORD lastError = GetLastError();
	if (lastError != ERROR_NO_MORE_FILES) {
		throw logError(lastError);
	}

	return fileList;
}

std::wstring multiByteToWstring(char const *input, UINT codepage) {
	if (input[0] == '\0') {
		return std::wstring();
	}

	int requiredLength = MultiByteToWideChar(
		codepage,
		0,
		input,
		-1,
		NULL,
		0
	);

	if (requiredLength == 0) {
		throw logError(GetLastError());
	}

	std::shared_ptr<wchar_t> output(new wchar_t[requiredLength], std::default_delete<wchar_t[]>());

	int result = MultiByteToWideChar(
		codepage,
		0,
		input,
		-1,
		output.get(),
		requiredLength
	);

	if (result == 0) {
		throw logError(GetLastError());
	}

	return std::wstring(output.get());
}

std::string wstringToMultiByte(std::wstring const &input, UINT codepage) {
	if (input.empty()) {
		return std::string();
	}

	int requiredLength = WideCharToMultiByte(
		codepage,
		0,
		input.c_str(),
		-1,
		NULL,
		0,
		NULL,
		NULL
	);

	if (requiredLength == 0) {
		throw logError(GetLastError());
	}

	std::shared_ptr<char> output(new char[requiredLength], std::default_delete<char[]>());

	int result = WideCharToMultiByte(
		codepage,
		0,
		input.c_str(),
		-1,
		output.get(),
		requiredLength,
		NULL,
		NULL
	);

	if (result == 0) {
		throw logError(GetLastError());
	}

	return std::string(output.get());
}
