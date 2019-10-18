#include "stdafx.h"
#include "InputPlugin.h"

#include <iostream>
#include <memory>

#include <winamp/in2.h>

#include "util.h"
#include "OutputPlugin.h"

// even for unicode plugins, many fields are not unicode, and I didn't see anywhere that the codepage was defined, so let's hope everyone used codepage 1252 for non-unicode strings
static std::wstring winampStringToWstring(char const *input) {
	return multiByteToWstring(input, 1252);
}

typedef In_Module *(__stdcall *GetInModuleFn)(void);

// WinAmp functions that we don't use, but need to supply to plugins.
// Input plugins
struct InputPluginFunctions {
	static void SAVSAInit(int maxlatency_in_ms, int srate) {}
	static void SAVSADeInit() {}
	static void SAAddPCMData(void *PCMData, int nch, int bps, int timestamp) {}
	static int SAGetMode() { return 1; }
	static int SAAdd(void *data, int timestamp, int csa) { return 1; }
	static void VSAAddPCMData(void *PCMData, int nch, int bps, int timestamp) {}
	static int VSAGetMode(int *specNch, int *waveNch) { return 1; }
	static int VSAAdd(void *data, int timestamp) { return 1; }
	static void VSASetInfo(int srate, int nch) { }
	static int dsp_isactive() { return 0; }
	static int dsp_dosamples(short int *samples, int numsamples, int bps, int nch, int srate) { return 0; }
	static void SetInfo(int bitrate, int srate, int stereo, int synched) {}
};

InputPlugin::InputPlugin(HWND mainWindowHandle, std::wstring libraryPath, std::shared_ptr<OutputPlugin> outputPlugin)
	: _outputPlugin(outputPlugin)
{
	_pluginHandle = std::shared_ptr<HMODULE>(
		new HMODULE{ LoadLibraryW(libraryPath.c_str()) }, 
		[](HMODULE *ptr) { 
			FreeLibrary(*ptr); 
			delete ptr; 
		}
	);
	
	GetInModuleFn getInModuleFn = (GetInModuleFn)GetProcAddress(*_pluginHandle, "winampGetInModule2");

	_pluginModule = getInModuleFn();
	_pluginModule->hMainWindow = mainWindowHandle;
	_pluginModule->hDllInstance = *_pluginHandle;
	_pluginModule->SAVSAInit = InputPluginFunctions::SAVSAInit;
	_pluginModule->SAVSADeInit = InputPluginFunctions::SAVSADeInit;
	_pluginModule->SAAddPCMData = InputPluginFunctions::SAAddPCMData;
	_pluginModule->SAGetMode = InputPluginFunctions::SAGetMode;
	_pluginModule->SAAdd = InputPluginFunctions::SAAdd;
	_pluginModule->VSAAddPCMData = InputPluginFunctions::VSAAddPCMData;
	_pluginModule->VSAGetMode = InputPluginFunctions::VSAGetMode;
	_pluginModule->VSAAdd = InputPluginFunctions::VSAAdd;
	_pluginModule->VSASetInfo = InputPluginFunctions::VSASetInfo;
	_pluginModule->dsp_isactive = InputPluginFunctions::dsp_isactive;
	_pluginModule->dsp_dosamples = InputPluginFunctions::dsp_dosamples;
	_pluginModule->SetInfo = InputPluginFunctions::SetInfo;
	_pluginModule->outMod = outputPlugin->getOutModule();

	_pluginModule->Init();

	parseExtensions();
}

In_Module *InputPlugin::pluginModule() {
	return _pluginModule;
}

std::wstring InputPlugin::description() {
	return winampStringToWstring(_pluginModule->description);
}

bool InputPlugin::isUnicode() {
	return (_pluginModule->version & IN_UNICODE) == IN_UNICODE;
}

std::list<PluginFileExtension> InputPlugin::extensions() {
	return _extensions;
}

PluginFileInfo InputPlugin::getFileInfo(std::wstring const &path) {
	// for non-unicode plugins, we use the ANSI charset for the filename, but windows-1252 for the title

	int lengthInMs;
	
	std::shared_ptr<void> title_c;

	if (isUnicode()) {
		title_c.reset(new wchar_t[GETFILEINFO_TITLE_LENGTH], std::default_delete<wchar_t[]>());
	}
	else {
		title_c.reset(new char[GETFILEINFO_TITLE_LENGTH], std::default_delete<char[]>());
	}

	std::shared_ptr<void> path_c(getMaybeMultiByteString(path, CP_ACP));

	_pluginModule->GetFileInfo(
		reinterpret_cast<char const *>(path_c.get()),
		reinterpret_cast<char *>(title_c.get()),
		&lengthInMs
	);

	std::wstring title = loadMaybeUnicodeString(title_c.get(), 1252);

	return PluginFileInfo{ title, lengthInMs };
}

void InputPlugin::playFile(std::wstring const &path) {
	std::shared_ptr<void> path_c(getMaybeMultiByteString(path, CP_ACP));
	_pluginModule->Play(reinterpret_cast<char const *>(path_c.get()));
}

std::wstring InputPlugin::loadMaybeUnicodeString(void const *str, UINT codepage) {
	if (isUnicode()) {
		return std::wstring(reinterpret_cast<wchar_t const *>(str));
	}
	else {
		return multiByteToWstring(reinterpret_cast<char const *>(str), codepage);
	}
}

std::shared_ptr<void> InputPlugin::getMaybeMultiByteString(std::wstring const &str, UINT codepage) {
	if (isUnicode()) {
		return std::shared_ptr<void>(_wcsdup(str.c_str()), free);
	} else {
		return std::shared_ptr<void>(_strdup(wstringToMultiByte(str, codepage).c_str()), free);
	}
}

void InputPlugin::parseExtensions() {
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

InputPlugin::~InputPlugin()
{
	_pluginModule->Quit();
	FreeLibrary(*_pluginHandle);
}
