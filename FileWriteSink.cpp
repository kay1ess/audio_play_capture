#include "FileWriteSink.h"
#include <iostream>
#include <fstream>

FileWriteSink::FileWriteSink(int channels, int bid_depth, const char* pcm_path)
{
	m_channels = channels;
	m_bit_depth = bid_depth;
	m_done = FALSE;
	m_file.open(pcm_path, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!m_file.is_open()) {
		printf("[ERROR] file open failed!\n");
		exit(-2);
	}
}


FileWriteSink::~FileWriteSink()
{
	m_file.close();
}

int FileWriteSink::CopyData(const BYTE* data, const int frames, int* bDone)
{

	int bytesPerSample = m_bit_depth / 8;
	UINT byteCount = frames * bytesPerSample * m_channels;

	m_file.write((const char *)data, byteCount);
	HRESULT notImpl = 0;
	return notImpl;
}
