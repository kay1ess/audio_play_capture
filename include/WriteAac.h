#pragma once

#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <audiopolicy.h>
#include <iostream>
#include <fstream>
#include "IAudioSink.h"
#include "faac.h"

class AACWriter : public IAudioSink
{
	std::ofstream m_file;

	int m_channels;
	int m_bit_depth;
	int m_sample_rate;
	int m_bit_rate;
	int m_done;
public:
	AACWriter(int channels, int bit_depth, int sample_rate, int bit_rate, const char* dst_path);
	~AACWriter();
	int CopyData(const BYTE*, const int, int*) override;
private:
	faacEncHandle m_encoder;
	DWORD m_max_output_bytes;
	int m_pcm_buffer_size;
	BYTE* m_pcm_buf;
	BYTE* m_aac_buf;
	int m_byte_count;
	int m_start;
	int m_exceed;
	DWORD m_input_samples;

};