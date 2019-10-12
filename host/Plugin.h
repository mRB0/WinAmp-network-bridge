#pragma once

#include <string>
#include <list>
#include <memory>
#include <Winamp/IN2.H>

struct PluginFileExtension {
	std::wstring extension;
	std::wstring description;
};

struct PluginFileInfo {
	std::wstring title;
	int lengthInMs;
};

class Plugin
{
public:
	Plugin(HWND mainWindowHandle, std::wstring libraryPath);
	~Plugin();

	In_Module const *pluginModule();

	std::wstring description();
	bool isUnicode();

	std::list<PluginFileExtension> extensions();
	PluginFileInfo getFileInfo(std::wstring const &path);

private:
	HMODULE _pluginHandle;
	In_Module *_pluginModule;
	std::list<PluginFileExtension> _extensions;

	void parseExtensions();

	// Load a string from the plugin that's interpreted as unicode if the plugin is a unicode plugin; otherwise, you need to specify what codepage to expect.
	// str should be either (wchar_t *) or (char *), or (in_char) using the WinAmp plugin notation.
	// codepage is the same as the MultiByteToWideChar argument with a similar name.
	std::wstring loadMaybeUnicodeString(void const *str, UINT codepage);

	// Get a string to be passed to the plugin.
	// The managed pointer is a (wchar_t *) if the plugin is a unicode plugin, or (char *) otherwise; aka (in_char) using the WinAmp plugin notation.
	// codepage is the same as the WideCharToMultiByte argument with a similar name.
	std::shared_ptr<void> getMaybeMultiByteString(std::wstring const &str, UINT codepage);
};
