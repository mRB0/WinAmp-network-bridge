#include "stdafx.h"
#include "Plugin.h"

#include <iostream>
#include <locale>
#include <codecvt>

#include <winamp/in2.h>

#include "util.h"

// even for unicode plugins, many fields are not unicode, and I didn't see anywhere that the codepage was defined, so let's hope everyone used codepage 1252 for non-unicode strings
static std::wstring winampStringToWstring(char const *input) {
	return multiByteToWstring(input, 1252);
}

typedef In_Module *(__stdcall *GetInModuleFn)(void);

Plugin::Plugin(HWND mainWindowHandle, std::wstring libraryPath)
{
	pluginHandle = LoadLibraryW(libraryPath.c_str());
	GetInModuleFn getInModuleFn = (GetInModuleFn)GetProcAddress(pluginHandle, "winampGetInModule2");

	pluginModule = getInModuleFn();
	pluginModule->hMainWindow = mainWindowHandle;
	pluginModule->hDllInstance = pluginHandle;

	pluginModule->Init();
}

In_Module const *Plugin::getPluginModule() {
	return pluginModule;
}

std::wstring Plugin::description() {
	return winampStringToWstring(pluginModule->description);
}

bool Plugin::isUnicode() {
	return (pluginModule->version & IN_UNICODE) == IN_UNICODE;
}

std::list<PluginFileExtension> Plugin::extensions() {
	// from the header file:
	//     char *FileExtensions;		// "mp3\0Layer 3 MPEG\0mp2\0Layer 2 MPEG\0mpg\0Layer 1 MPEG\0"
	// there's an implied additional null-terminator to indicate the end of the list, and many plugins use ; to handle multiple file types, eg. "vgm;vgz\0VGM file\0"

	auto extensionsList = std::list<PluginFileExtension>();
	size_t offset = 0;

	for (;;) {
		char *extensions_c = pluginModule->FileExtensions + offset;

		if (*extensions_c == '\0') {
			return extensionsList;
		}

		offset += strlen(extensions_c) + 1;
		char *description_c = pluginModule->FileExtensions + offset;
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
	pluginModule->Quit();
	FreeLibrary(pluginHandle);
}
