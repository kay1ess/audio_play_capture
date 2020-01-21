#pragma once
#include "IAudioSink.h"
#include <fstream>
#include "faad.h"

class AACReader : public IAudioSink
{
	std::ifstream m_file;

	int m_channels;
	int m_bit_depth;
	int m_sample_rate;
	NeAACDecHandle m_decoder;
	NeAACDecFrameInfo m_info;
	BYTE* m_aac_buf;
	BYTE* m_pcm_buf;
	int m_pcm_ptr_pos;
	int m_exceed;
	BYTE* m_exceed_buf;
	BYTE* m_frame;
	int m_read_byte;
	int m_aac_size;
	BYTE* m_temp_ptr;
	BYTE* m_temp_file_buf;
	bool m_first;
	int m_done;
	int m_size;
	int m_offset;
public:
	AACReader(int Channels, int BitDepth, int SampleRate, const char* dst_path);
	int get_one_ADTS_frame(BYTE* buffer, int buf_size, BYTE* data, int* data_size);
	~AACReader();
	int LoadData(const int frames,const BYTE* data, int* bDone) override;
private:
};