#include "stdafx.h"
#include "Plugin.h"

#include <iostream>

#include <winamp/in2.h>

#include "util.h"
#include "finally.h"

// even for unicode plugins, many fields are not unicode, and I didn't see anywhere that the codepage was defined, so let's hope everyone used codepage 1252 for non-unicode strings
static std::wstring winampStringToWstring(char const *input) {
	return multiByteToWstring(input, 1252);
}

typedef In_Module *(__stdcall *GetInModuleFn)(void);

Plugin::Plugin(HWND mainWindowHandle, std::wstring libraryPath)
{
	_pluginHandle = LoadLibraryW(libraryPath.c_str());
	GetInModuleFn getInModuleFn = (GetInModuleFn)GetProcAddress(_pluginHandle, "winampGetInModule2");

	_pluginModule = getInModuleFn();
	_pluginModule->hMainWindow = mainWindowHandle;
	_pluginModule->hDllInstance = _pluginHandle;

	parseExtensions();

	_pluginModule->Init();
}

In_Module const *Plugin::pluginModule() {
	return _pluginModule;
}

std::wstring Plugin::description() {
	return winampStringToWstring(_pluginModule->description);
}

bool Plugin::isUnicode() {
	return (_pluginModule->version & IN_UNICODE) == IN_UNICODE;
}

std::list<PluginFileExtension> Plugin::extensions() {
	return _extensions;
}

PluginFileInfo Plugin::getFileInfo(std::wstring const &path) {
	// for non-unicode plugins, we use the ANSI charset for the filename, but windows-1252 for the title

	int lengthInMs;
	
	void *title_c;

	if (isUnicode()) {
		title_c = new wchar_t[GETFILEINFO_TITLE_LENGTH];
	}
	else {
		title_c = new char[GETFILEINFO_TITLE_LENGTH];
	}

	auto _deleteTitle_c = finally([title_c] { delete[] title_c; });

	void *path_c = getMaybeMultiByteString(path, CP_ACP);
	auto _freePath_c = finally([path_c] { free(path_c); });

	_pluginModule->GetFileInfo(
		reinterpret_cast<char const *>(path_c),
		reinterpret_cast<char *>(title_c),
		&lengthInMs
	);

	std::wstring title = loadMaybeUnicodeString(title_c, 1252);

	return PluginFileInfo{ title, lengthInMs };
}

std::wstring Plugin::loadMaybeUnicodeString(void const *str, UINT codepage) {
	if (isUnicode()) {
		return std::wstring(reinterpret_cast<wchar_t const *>(str));
	}
	else {
		return multiByteToWstring(reinterpret_cast<char const *>(str), codepage);
	}
}

void * Plugin::getMaybeMultiByteString(std::wstring const &str, UINT codepage) {
	if (isUnicode()) {
		return _wcsdup(str.c_str());
	} else {
		return _strdup(wstringToMultiByte(str, codepage).c_str());
	}
}

void Plugin::parseExtensions() {
	// from the header file:
	//     char *FileExtensions;		// "mp3\0Layer 3 MPEG\0mp2\0Layer 2 MPEG\0mpg\0Layer 1 MPEG\0"
	// there's an implied additional null-terminator to indicate the end of the list, and many plugins use ; to handle multiple file types, eg. "vgm;vgz\0VGM file\0"

	auto extensionsList = std::list<PluginFileExtension>();
	size_t offset = 0;

	for (;;) {
		char *extensions_c = _pluginModule->FileExtensions + offset;

		if (*extensions_c == '\0') {
			_extensions = extensionsList;
			return;
		}

		offset += strlen(extensions_c) + 1;
		char *description_c = _pluginModule->FileExtensions + offset;
		offset += strlen(description_c) + 1;


		std::wstring semicolonDelimitedExtensions(winampStringToWstring(extensions_c));
		std::wstring description(winampStringToWstring(description_c));

		size_t nextSearchStartPos = 0;

		size_t delimiterPos;
		while ((delimiterPos = semicolonDelimitedExtensions.find(L";", nextSearchStartPos)) != std::string::npos) {
			std::wstring extension = semicolonDelimitedExtensions.substr(nextSearchStartPos, delimiterPos - nextSearchStartPos);
			extensionsList.push_back(PluginFileExtension{ extension, description });

			nextSearchStartPos = delimiterPos + 1;
		}

		extensionsList.push_back(PluginFileExtension{ semicolonDelimitedExtensions.substr(nextSearchStartPos), description });
	}
}

Plugin::~Plugin()
{
	_pluginModule->Quit();
	FreeLibrary(_pluginHandle);
}
