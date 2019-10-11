#pragma once

#include <list>
#include <string>

#include "wxx_wincore.h"

#define odslog(msg) { std::wstringstream ss; ss << msg; OutputDebugStringW(ss.str().c_str()); }

std::wstring getErrorDescription(DWORD error);

std::string copyWstringToStringSameEncoding(std::wstring const &wstring);

std::exception logError(DWORD error);

std::list<std::wstring> listFiles(std::wstring const &directory, std::wstring const &fileSpec);

std::wstring multiByteToWstring(char const *input, UINT codepage);
