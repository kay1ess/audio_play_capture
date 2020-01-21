
#include "WriteAac.h"


AACWriter::AACWriter(int channels, int bit_depth, int sample_rate, int bit_rate, const char* dst_path) {
	m_bit_depth = bit_depth;
	m_channels = channels;
	m_sample_rate = sample_rate;
	m_bit_rate = bit_rate;

	m_encoder = faacEncOpen(m_sample_rate, m_channels, &m_input_samples, &m_max_output_bytes);
	if (!m_encoder)
	{
		printf("[ERROR] Failed to call faacEncOpen()\n");
		exit(-1);
	}
	m_pcm_buffer_size = m_input_samples * m_bit_depth / 8;

	// 编码器设置
	faacEncConfigurationPtr pConfiguration = faacEncGetCurrentConfiguration(m_encoder);
	pConfiguration->inputFormat = FAAC_INPUT_16BIT;
	pConfiguration->bitRate = m_bit_rate;
	pConfiguration->useTns = 1;
	pConfiguration->outputFormat = 1;
	pConfiguration->aacObjectType = LOW;
	pConfiguration->mpegVersion = 4; // MPEG4
	pConfiguration->useTns = 1;
	faacEncSetConfiguration(m_encoder, pConfiguration);

	m_file.open(dst_path, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!m_file.is_open()) {
		printf("[ERROR] file open failed!\n");
		exit(-2);
	}
	m_pcm_buf = new BYTE[m_pcm_buffer_size];
	m_aac_buf = new BYTE[m_max_output_bytes];
	m_start = 0;
	m_done = FALSE;
}


AACWriter::~AACWriter() {
	faacEncClose(m_encoder);
	if (m_pcm_buf != NULL) {
		delete[] m_pcm_buf;
		m_pcm_buf = NULL;
	}
	faacEncClose(m_encoder);
	if (m_aac_buf != NULL) {
		delete[] m_aac_buf;
		m_aac_buf = NULL;
	}
	m_file.close();
}


int AACWriter::CopyData(const BYTE* data, int frames, int* bDone) {

	// 通过控制m_done 来控制bDone
	// 可以另起一个线程进行控制
	m_done = FALSE;
	bDone = &m_done;

	UINT byteCount = frames * (m_bit_depth/8) * m_channels;

	int len = strlen((char*)data);
	if (len == 0) {
		HRESULT notImpl = 0;
		return notImpl;
	}

	m_exceed = (m_start + byteCount) - m_pcm_buffer_size;
	// 缓冲区溢出
	if (m_exceed > 0) {

		// 将多余的数据写入缓冲区 此时缓冲区正好满了
		int fill_bytes = m_pcm_buffer_size - m_start;
		memcpy(m_pcm_buf + m_start, data, fill_bytes);
		m_byte_count = faacEncEncode(
			m_encoder, (int*)m_pcm_buf, m_input_samples, m_aac_buf, m_max_output_bytes);

		if (m_byte_count > 0) {
			m_file.write((const char*)m_aac_buf, m_byte_count);
		}

		// 编码后重置偏移量
		m_start = 0;
		// 此时缓冲区再去拷贝剩余的数据
		memcpy(m_pcm_buf + m_start, data + fill_bytes, m_exceed);
		m_start += m_exceed;
	}
	else {
		memcpy(m_pcm_buf + m_start, data, byteCount);
		m_start += byteCount;
	}
	
	HRESULT notImpl = 0;
	return notImpl;
}




