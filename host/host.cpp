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

#include "wxx_wincore.h"

#include "Plugin.h"
#include "finally.h"
#include <winamp/in2.h>
#include "util.h"

typedef In_Module *(__stdcall *GetInModuleFn)(void);

static std::shared_ptr<Plugin> findPluginForExtension(std::list<std::shared_ptr<Plugin>> plugins, std::wstring const &extension) {
	for (auto const &plugin : plugins) {
		
		for (auto const &pluginExtension : plugin->extensions()) {
			if (lstrcmpiW(extension.c_str(), pluginExtension.extension.c_str()) == 0) {
				return plugin;
			}
		}
		
	}

	return nullptr;
}

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
			odslog(L"    uses output plug: " << plugin->pluginModule()->UsesOutputPlug << std::endl);
			odslog(L"    is seekable: " << plugin->pluginModule()->is_seekable << std::endl);
			odslog(L"    file extensions:" << std::endl);

			for (auto const &pluginExtension : plugin->extensions()) {
				odslog(L"        " << pluginExtension.extension << L" (" << pluginExtension.description << L")" << std::endl);
			}

			odslog(L"    functions:" << std::endl);

			// functions provided by plugin
			odslog(L"        SetVolume = 0x" << plugin->pluginModule()->SetVolume << std::endl);
			odslog(L"        SetPan = 0x" << plugin->pluginModule()->SetPan << std::endl);
			odslog(L"        EQSet = 0x" << plugin->pluginModule()->EQSet << std::endl);

			// functions provided by host
			odslog(L"        SAVSAInit = 0x" << plugin->pluginModule()->SAVSAInit << std::endl);
			odslog(L"        SAVSADeInit = 0x" << plugin->pluginModule()->SAVSADeInit << std::endl);
			odslog(L"        SAAddPCMData = 0x" << plugin->pluginModule()->SAAddPCMData << std::endl);
			odslog(L"        SAGetMode = 0x" << plugin->pluginModule()->SAGetMode << std::endl);
			odslog(L"        SAAdd = 0x" << plugin->pluginModule()->SAAdd << std::endl);
			odslog(L"        VSAAddPCMData = 0x" << plugin->pluginModule()->VSAAddPCMData << std::endl);
			odslog(L"        VSAGetMode = 0x" << plugin->pluginModule()->VSAGetMode << std::endl);
			odslog(L"        VSAAdd = 0x" << plugin->pluginModule()->VSAAdd << std::endl);
			odslog(L"        VSASetInfo = 0x" << plugin->pluginModule()->VSASetInfo << std::endl);
			odslog(L"        dsp_isactive = 0x" << plugin->pluginModule()->dsp_isactive << std::endl);
			odslog(L"        dsp_dosamples = 0x" << plugin->pluginModule()->dsp_dosamples << std::endl);
			odslog(L"        SetInfo = 0x" << plugin->pluginModule()->SetInfo << std::endl);
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
