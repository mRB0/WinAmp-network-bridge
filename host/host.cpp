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
#include <winamp/in2.h>
#include "util.h"

typedef In_Module *(__stdcall *GetInModuleFn)(void);

static std::shared_ptr<Plugin> findPluginForExtension(std::list<std::shared_ptr<Plugin>> const &plugins, std::wstring const &extension) {
	for (auto const &plugin : plugins) {
		
		for (auto const &pluginExtension : plugin->extensions()) {
			if (lstrcmpiW(extension.c_str(), pluginExtension.extension.c_str()) == 0) {
				return plugin;
			}
		}
		
	}

	return nullptr;
}

// WinAmp functions that we don't need, but need to supply to plugins.
// Each plugin is treated as a singleton: They're implemented using C function pointers with no context data, so we can only have one instance of each output plugin that we implement.

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

// Output plugins
struct OutputPluginFunctions {
	static void Config(HWND hwndParent) {}
	static void About(HWND hwndParent) {}
	static void Init() {}
	static void Quit() {}

	static int Open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms) {
		odslog(L"Output::Open(): samplerate = " << samplerate << ", numchannels = " << numchannels << ", bitspersamp = " << bitspersamp << ", bufferlenms = " << bufferlenms << ", prebufferms = " << prebufferms << std::endl);
		return 0;
	}
	
	static void Close() {
		odslog(L"Output::Close()" << std::endl);
	}

	static int Write(char *buf, int len) {
		odslog(L"Output::Write: len = " << len << std::endl);
		return 0;
	}

	static int CanWrite() {
		odslog(L"Output::CanWrite" << std::endl);
		return 8192;
	}

	static int IsPlaying() {
		odslog(L"Output::IsPlaying" << std::endl);
		return 0;
	}

	static int Pause(int pause) {
		odslog(L"Output::Pause" << std::endl);
		return 0;
	}

	static void SetVolume(int volume) {
		odslog(L"Output::SetVolume: volume = " << volume << std::endl);
	}

	static void SetPan(int pan) {
		odslog(L"Output::SetPan: pan = " << pan << std::endl);
	}

	static void Flush(int t) {
		odslog(L"Output::Flush: t = " << t << std::endl);
	}

	static int GetOutputTime() {
		odslog(L"Output::GetOutputTime" << std::endl);
		return 0;
	}

	static int GetWrittenTime() {
		odslog(L"Output::GetWrittenTime" << std::endl);
		return 0;
	}
};

class CView : public CWnd
{
public:

	CView() {
		outMod.version = OUT_VER;
		outMod.description = "WinAmp Network Bridge Output Plugin";
		outMod.id = 70834;
		outMod.hMainWindow = GetHwnd();
		outMod.hDllInstance = GetModuleHandle(NULL);
		outMod.Config = OutputPluginFunctions::Config;
		outMod.About = OutputPluginFunctions::About;
		outMod.Init = OutputPluginFunctions::Init;
		outMod.Quit = OutputPluginFunctions::Quit;
		outMod.Open = OutputPluginFunctions::Open;
		outMod.Close = OutputPluginFunctions::Close;
		outMod.Write = OutputPluginFunctions::Write;
		outMod.CanWrite = OutputPluginFunctions::CanWrite;
		outMod.IsPlaying = OutputPluginFunctions::IsPlaying;
		outMod.Pause = OutputPluginFunctions::Pause;
		outMod.SetVolume = OutputPluginFunctions::SetVolume;
		outMod.SetPan = OutputPluginFunctions::SetPan;
		outMod.Flush = OutputPluginFunctions::Flush;
		outMod.GetOutputTime = OutputPluginFunctions::GetOutputTime;
		outMod.GetWrittenTime = OutputPluginFunctions::GetWrittenTime;

		std::wstring const pluginPath(L"C:\\Users\\mrb\\Documents\\code\\winamp-network-bridge\\plugins");
		std::wstring const fileSpec(L"\\in_*.dll");

		auto pluginFiles = listFiles(pluginPath, fileSpec);

		std::shared_ptr<std::list<std::shared_ptr<Plugin>>> plugins(new std::list<std::shared_ptr<Plugin>>);

		for (auto const &filename : pluginFiles) {
			auto plugin = std::make_shared<Plugin>(this->GetHwnd(), filename);
			plugin->pluginModule()->SAVSAInit = InputPluginFunctions::SAVSAInit;
			plugin->pluginModule()->SAVSADeInit = InputPluginFunctions::SAVSADeInit;
			plugin->pluginModule()->SAAddPCMData = InputPluginFunctions::SAAddPCMData;
			plugin->pluginModule()->SAGetMode = InputPluginFunctions::SAGetMode;
			plugin->pluginModule()->SAAdd = InputPluginFunctions::SAAdd;
			plugin->pluginModule()->VSAAddPCMData = InputPluginFunctions::VSAAddPCMData;
			plugin->pluginModule()->VSAGetMode = InputPluginFunctions::VSAGetMode;
			plugin->pluginModule()->VSAAdd = InputPluginFunctions::VSAAdd;
			plugin->pluginModule()->VSASetInfo = InputPluginFunctions::VSASetInfo;
			plugin->pluginModule()->dsp_isactive = InputPluginFunctions::dsp_isactive;
			plugin->pluginModule()->dsp_dosamples = InputPluginFunctions::dsp_dosamples;
			plugin->pluginModule()->SetInfo = InputPluginFunctions::SetInfo;
			plugin->pluginModule()->outMod = &outMod;

			plugins->push_back(plugin);
			
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

		this->plugins = plugins;

		playFile(L"K:\\temp\\test.vgz");
	}

	virtual ~CView() {}
	virtual void OnDestroy() { PostQuitMessage(0); }	// Ends the application

	virtual void OnCreate() {

	}

private:

	Out_Module outMod{};

	std::wstring getFileExtension(std::wstring const &path) {
		LPCWSTR pExtension = PathFindExtensionW(path.c_str());
		if (!pExtension) {
			throw std::exception("File has no extension :(");
		}

		return std::wstring(pExtension + 1); // skip the '.'
	}

	void playFile(std::wstring path) {
		auto plugin = findPluginForExtension(*plugins, getFileExtension(path));
		plugin->playFile(path);
	}

	std::shared_ptr<std::list<std::shared_ptr<Plugin>>> plugins;
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
