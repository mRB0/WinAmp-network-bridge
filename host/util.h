#pragma once

#include <string>

#include "wxx_wincore.h"

#define odslog(msg) { std::wstringstream ss; ss << msg; OutputDebugStringW(ss.str().c_str()); }

std::wstring getErrorDescription(DWORD error);

std::string logError(DWORD error);

std::wstring multiByteToWstring(char const *input, UINT codepage);

std::string wstringToMultiByte(std::wstring const &input, UINT codepage);
