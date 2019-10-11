// host.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "host.h"
#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <memory>
#include <list>
#include <locale>
#include <codecvt>

#include "wxx_wincore.h"

#include "Plugin.h"
#include "finally.h"
#include <winamp/in2.h>
#include "util.h"

typedef In_Module *(__stdcall *GetInModuleFn)(void);

class CView : public CWnd
{
public:

	CView() {
		std::wstring const pluginPath(L"C:\\Users\\mrb\\Documents\\code\\winamp-network-bridge\\plugins");
		std::wstring const fileSpec(L"\\in_*.dll");

		auto pluginFiles = listFiles(pluginPath, fileSpec);
		
		std::list<std::shared_ptr<Plugin>> plugins;

		for (auto const &filename : pluginFiles) {
			auto plugin = std::make_shared<Plugin>(this->GetHwnd(), filename);
			plugins.push_back(plugin);
			
			odslog(filename << L": " << plugin->description() << std::endl);
			odslog(L"    unicode: " << plugin->isUnicode() << std::endl);
			odslog(L"    uses output plug: " << plugin->getPluginModule()->UsesOutputPlug << std::endl);
			odslog(L"    file extensions:" << std::endl);

			for (auto const &pluginExtension : plugin->extensions()) {
				odslog(L"        " << pluginExtension.extension << L" (" << pluginExtension.description << L")" << std::endl);
			}
		}
	}

	virtual ~CView() {}
	virtual void OnDestroy() { PostQuitMessage(0); }	// Ends the application

	virtual void OnCreate() {

	}
};


int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// Start Win32++
	CWinApp app;

	// Create our view window
	CView mainWindow;
	mainWindow.Create();

	// Run the application
	return app.Run();
}
