#include "stdafx.h"
#include "util.h"

#include <memory>
#include <ios>
#include <stdexcept>

#include <win32xx/wxx_wincore.h>

std::wstring getErrorDescription(DWORD error) {
	CStringW str;
	str.GetErrorString(error);

	return std::wstring(str.c_str());
}

std::string logError(DWORD error) {
	auto errorDescription = getErrorDescription(error);
	odslog(L"Error: " << errorDescription << std::endl);

	return wstringToMultiByte(errorDescription, 1252);
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
		throw std::invalid_argument(logError(GetLastError()));
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
		throw std::invalid_argument(logError(GetLastError()));
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
		throw std::invalid_argument(logError(GetLastError()));
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
		throw std::invalid_argument(logError(GetLastError()));
	}

	return std::string(output.get());
}
