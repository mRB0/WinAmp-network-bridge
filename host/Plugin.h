#pragma once

#include <string>
#include <list>
#include <Winamp/IN2.H>

struct PluginFileExtension {
	std::wstring extension;
	std::wstring description;
};

class Plugin
{
public:
	Plugin(HWND mainWindowHandle, std::wstring libraryPath);
	~Plugin();

	In_Module const *getPluginModule();

	std::wstring description();
	bool isUnicode();

	std::list<PluginFileExtension> extensions();

private:
	HMODULE pluginHandle;
	In_Module *pluginModule;
};
