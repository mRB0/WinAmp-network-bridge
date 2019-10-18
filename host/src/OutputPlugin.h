#pragma once

#include "winamp/in2.h"

class OutputPlugin
{
public:
	virtual ~OutputPlugin() {}

	// return >=0 on success, <0 on failure
	virtual int open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms) = 0;
	virtual void close() = 0;
	// len <= 8192
	// return 0 on success, 1 if not yet able to write
	virtual int write(char *buf, int len) = 0;
	// return number of bytes that can be written
	virtual int canWrite() = 0;
	// return 0 if not playing (ie. audio buffers have been flushed), non-zero if audio is still being played
	virtual int isPlaying() = 0;
	// return last pause state
	virtual int pause(int pause) = 0;
	// volume is in [0, 255]
	virtual void setVolume(int volume) = 0;
	// pan is from [-128, 128]
	virtual void setPan(int pan) = 0;
	// flushes buffers and restarts output at time t (in ms) (used for seeking)
	virtual void flush(int t) = 0;
	// return played time in ms
	virtual int getOutputTime() = 0;
	// return time written in ms (used for synching up vis stuff)
	virtual int getWrittenTime() = 0;

	virtual Out_Module *getOutModule() = 0;
};
