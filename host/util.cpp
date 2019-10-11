#include "stdafx.h"
#include "util.h"

#include "wxx_wincore.h"

#include <locale>
#include <codecvt>
#include "finally.h"

std::wstring getErrorDescription(DWORD error) {
	CStringW str;
	str.GetErrorString(error);

	return std::wstring(str.c_str());
}

std::string copyWstringToStringSameEncoding(std::wstring const &wstring) {
	using convertType = std::codecvt_utf16<wchar_t>;

	std::wstring_convert<convertType, wchar_t> converter;
	return std::string(converter.to_bytes(wstring));
}

std::exception logError(DWORD error) {
	auto errorDescription = getErrorDescription(error);
	odslog(L"Error: " << errorDescription << std::endl);

	return std::exception(copyWstringToStringSameEncoding(errorDescription).c_str());
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

	auto _deleter = finally([handle] { FindClose(handle); });

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

	auto output = new wchar_t[requiredLength];
	auto _outputDeleter = finally([output] { delete[] output; });

	int result = MultiByteToWideChar(
		codepage,
		0,
		input,
		-1,
		output,
		requiredLength
	);

	if (result == 0) {
		throw logError(GetLastError());
	}

	return std::wstring(output);
}