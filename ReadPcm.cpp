#include "ReadPcm.h"
#include <Audioclient.h>


PCMReader::PCMReader(int channels, int bit_depth, const char* pcm_path) {
	m_channels = channels;
	m_bit_depth = bit_depth;
	m_file.open(pcm_path, std::ios::in | std::ios::binary);
	if (!m_file.is_open()) {
		printf("[ERROR] file open failed!\n");
		exit(-2);
	}
}


PCMReader::~PCMReader() {
	m_file.close();
}


int PCMReader::LoadData(const int frames, const BYTE* data, int* bDone) {
	// 要送入音频设备的字节数
	int byte_count = m_channels * (m_bit_depth / 8)  * frames;
	
	if (m_file.eof()) {
		*bDone = TRUE;
	}
	else {
		m_file.read((char*)data, byte_count);
		*bDone = FALSE;
	}
	
	HRESULT notImpl = 0;
	return notImpl;
}