#include "stdafx.h"
#include "RPCOutputPlugin.h"
#include "util.h"

extern Out_Module singletonOutModule;

std::shared_ptr<RPCOutputPlugin> RPCOutputPlugin::setSingletonOutputPlugin(std::shared_ptr<RPCOutputPlugin> &newOutputPlugin) {
	auto oldSingleton = _singletonOutputPlugin;
	_singletonOutputPlugin = newOutputPlugin;
	return oldSingleton;
}

std::shared_ptr<RPCOutputPlugin> RPCOutputPlugin::getSingletonOutputPlugin() {
	return _singletonOutputPlugin;
}

Out_Module *RPCOutputPlugin::getSingletonOutModule() {
	return &singletonOutModule;
}

RPCOutputPlugin::RPCOutputPlugin()
{
}

RPCOutputPlugin::~RPCOutputPlugin()
{
}

int RPCOutputPlugin::open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms) {
	odslog(L"RPCOutputPlugin::open(): samplerate = " << samplerate << ", numchannels = " << numchannels << ", bitspersamp = " << bitspersamp << ", bufferlenms = " << bufferlenms << ", prebufferms = " << prebufferms << std::endl);
	return 0;
}

void RPCOutputPlugin::close() {
	odslog(L"Output::Close()" << std::endl);
}

int RPCOutputPlugin::write(char *buf, int len) {
	odslog(L"Output::Write: len = " << len << std::endl);
	return 0;
}

int RPCOutputPlugin::canWrite() {
	odslog(L"Output::CanWrite" << std::endl);
	return 8192;
}

int RPCOutputPlugin::isPlaying() {
	odslog(L"Output::IsPlaying" << std::endl);
	return 0;
}

int RPCOutputPlugin::pause(int pause) {
	odslog(L"Output::Pause" << std::endl);
	return 0;
}

void RPCOutputPlugin::setVolume(int volume) {
	odslog(L"Output::SetVolume: volume = " << volume << std::endl);
}

void RPCOutputPlugin::setPan(int pan) {
	odslog(L"Output::SetPan: pan = " << pan << std::endl);
}

void RPCOutputPlugin::flush(int t) {
	odslog(L"Output::Flush: t = " << t << std::endl);
}

int RPCOutputPlugin::getOutputTime() {
	odslog(L"Output::GetOutputTime" << std::endl);
	return 0;
}

int RPCOutputPlugin::getWrittenTime() {
	odslog(L"Output::GetWrittenTime" << std::endl);
	return 0;
}

std::shared_ptr<RPCOutputPlugin> RPCOutputPlugin::_singletonOutputPlugin;

// statics

struct OutputPluginFunctions {
	static void Config(HWND hwndParent) {}
	static void About(HWND hwndParent) {}
	static void Init() {}
	static void Quit() {}

	static int Open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms) {
		return RPCOutputPlugin::getSingletonOutputPlugin()->open(samplerate, numchannels, bitspersamp, bufferlenms, prebufferms);
	}

	static void Close() {
		RPCOutputPlugin::getSingletonOutputPlugin()->close();
	}

	static int Write(char *buf, int len) {
		return RPCOutputPlugin::getSingletonOutputPlugin()->write(buf, len);
	}

	static int CanWrite() {
		return RPCOutputPlugin::getSingletonOutputPlugin()->canWrite();
	}

	static int IsPlaying() {
		return RPCOutputPlugin::getSingletonOutputPlugin()->isPlaying();
	}

	static int Pause(int pause) {
		return RPCOutputPlugin::getSingletonOutputPlugin()->pause(pause);
	}

	static void SetVolume(int volume) {
		RPCOutputPlugin::getSingletonOutputPlugin()->setVolume(volume);
	}

	static void SetPan(int pan) {
		RPCOutputPlugin::getSingletonOutputPlugin()->setPan(pan);
	}

	static void Flush(int t) {
		RPCOutputPlugin::getSingletonOutputPlugin()->flush(t);
	}

	static int GetOutputTime() {
		return RPCOutputPlugin::getSingletonOutputPlugin()->getOutputTime();
	}

	static int GetWrittenTime() {
		return RPCOutputPlugin::getSingletonOutputPlugin()->getWrittenTime();
	}
};

static Out_Module singletonOutModule{
	OUT_VER, // version
	"WinAmp Network Bridge Output Plugin", // description
	 70834, // id (>= 65536)
	NULL, // hMainWindow, should be supplied later
	NULL, // hDllInstance, should be supplied later

	OutputPluginFunctions::Config,
	OutputPluginFunctions::About,
	OutputPluginFunctions::Init,
	OutputPluginFunctions::Quit,
	OutputPluginFunctions::Open,
	OutputPluginFunctions::Close,
	OutputPluginFunctions::Write,
	OutputPluginFunctions::CanWrite,
	OutputPluginFunctions::IsPlaying,
	OutputPluginFunctions::Pause,
	OutputPluginFunctions::SetVolume,
	OutputPluginFunctions::SetPan,
	OutputPluginFunctions::Flush,
	OutputPluginFunctions::GetOutputTime,
	OutputPluginFunctions::GetWrittenTime,
};
