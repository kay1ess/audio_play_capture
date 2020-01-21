#pragma once
#include "IAudioSink.h"
#include <fstream>

class PCMReader : public IAudioSink
{
public:
	PCMReader(int Channels, int BitDepth, const char* pcm_path);
	~PCMReader();
	int LoadData(const int frames, const BYTE* data, int* bDone) override;
	

private:
	std::ifstream m_file;
	int m_channels;
	int m_bit_depth;

};