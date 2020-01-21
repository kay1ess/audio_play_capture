#pragma once

#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <audiopolicy.h>
#include <iostream>
#include <fstream>
#include "IAudioSink.h"

class FileWriteSink : public IAudioSink
{
	std::ofstream m_file;

	int m_channels;
	int m_bit_depth;
	int m_done;
public:
	FileWriteSink(int channels, int bit_depth, const char* pcm_path);
	~FileWriteSink();
	int CopyData(const BYTE*, const int, int*) override;
};
