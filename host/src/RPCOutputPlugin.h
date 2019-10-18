#pragma once

#include "OutputPlugin.h"
#include "winamp/in2.h"
#include <memory>

class RPCOutputPlugin : public OutputPlugin
{
public:
	RPCOutputPlugin();
	virtual ~RPCOutputPlugin();

	// return >=0 on success, <0 on failure
	virtual int open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms);
	virtual void close();
	// len <= 8192
	// return 0 on success, 1 if not yet able to write
	virtual int write(char *buf, int len);
	// return number of bytes that can be written
	virtual int canWrite();
	// return 0 if not playing (ie. audio buffers have been flushed), non-zero if audio is still being played
	virtual int isPlaying();
	// return last pause state
	virtual int pause(int pause);
	// volume is in [0, 255]
	virtual void setVolume(int volume);
	// pan is from [-128, 128]
	virtual void setPan(int pan);
	// flushes buffers and restarts output at time t (in ms) (used for seeking)
	virtual void flush(int t);
	// return played time in ms
	virtual int getOutputTime();
	// return time written in ms (used for synching up vis stuff)
	virtual int getWrittenTime();

	virtual Out_Module *getOutModule() { return getSingletonOutModule(); }

	// Each plugin is treated as a singleton: They're implemented using C function pointers with no context data, so we can only have one "active" instance of an output plugin.

	// Set the current RPCOutputPlugin as the active one. Returns the previous one.
	static std::shared_ptr<RPCOutputPlugin> setSingletonOutputPlugin(std::shared_ptr<RPCOutputPlugin> &newOutputPlugin);
	// Get the currently-active RPCOutputPlugin.
	static std::shared_ptr<RPCOutputPlugin> getSingletonOutputPlugin();

	static Out_Module *getSingletonOutModule();
	static void setOutModuleMainWindow(HWND mainWindow) { getSingletonOutModule()->hMainWindow = mainWindow; }
	static void setOutModuleDllInstance(HINSTANCE dllInstance) { getSingletonOutModule()->hDllInstance = dllInstance; }

private:
	static std::shared_ptr<RPCOutputPlugin> _singletonOutputPlugin;
};

