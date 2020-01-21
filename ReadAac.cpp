#include "ReadAcc.h"
#include "faad.h"
#include "neaacdec.h"
#include <Audioclient.h>
#include <iostream>

#define FRAME_MAX_LEN 1024 * 5
#define BUFFER_MAX_LEN 1024 * 50


AACReader::AACReader(int channels, int bit_depth, int sample_rate, const char* dst_path) {
	m_file.open(dst_path, std::ios::in|std::ios::binary);
	if (!m_file.is_open()) {
		printf("[ERROR] file open failed!\n");
		exit(-2);
	}

	m_channels = channels;
	m_bit_depth = bit_depth;
	m_sample_rate = sample_rate;

	m_pcm_ptr_pos = 0;							// 初始化指针偏移为0
	m_pcm_buf = NULL;
	m_exceed = 0;
	m_exceed_buf = new BYTE[FRAME_MAX_LEN];
	m_frame = new BYTE[FRAME_MAX_LEN];
	m_aac_buf = new BYTE[BUFFER_MAX_LEN];
	m_read_byte = 0;
	m_temp_ptr = nullptr;
	m_temp_file_buf = new BYTE[BUFFER_MAX_LEN*3];
	m_first = TRUE;
	m_aac_size = 0;
	m_size = 0;
	m_done = FALSE;
	m_offset = 0;
	// 初始化解码器
	m_decoder = NeAACDecOpen();
	if (!m_decoder)
	{
		printf("[ERROR] Failed to call NeAACDecOpen()\n");
		exit(-1);
	};

	
}

AACReader::~AACReader() {
	// 回收解码器资源
	if (m_decoder != NULL) {
		NeAACDecClose(m_decoder);
		m_decoder = NULL;
	}
	m_file.close();
}


// 获取一帧frame
int AACReader::get_one_ADTS_frame(BYTE* buffer, int buf_size, BYTE* data, int* data_size) {
	*data_size = 0;
	int size = 0;
	if (!buffer || !data || !data_size) {
		printf("[ERROR] dont get a adts frame\n");
		return -1;
	}
	m_offset = 0;

	while (1) {
		// 每个adts header固定为7个字节
		if (buf_size < 7) {
			return 1;
		}

		/*
		---------------------------------------------------------
		|		ADTS HEADER			|		BODY				|
		|			7Bytes			|		BODY				|
		---------------------------------------------------------
		*/

		// 找到同步位 总是以0xFFF开头 所以首先找到 0xFFF 
		// buffer[0] 两个字节  + buffer[1] & 0xF0 共三个字节
		if ((buffer[0] == 0xff) && ((buffer[1] & 0xf0) == 0xf0)) {
			// 找到同步位后 需要得到frame length 共13位 在31-44 bits之间
			// buffer[3] 8字节   buffer[3] * 0x03 取得31-32bits  
			size |= (((buffer[3] & 0x03)) << 11);	//high 2 bit
			// buffer[4] 取到 33bit-41bits
			size |= (buffer[4] << 3);				//middle 8 bit
			// buffer[5] 取到 42-44bits
			size |= ((buffer[5] & 0xe0) >> 5);		//low 3 bit
			break;
		}

		--(buf_size);
		++(buffer);
		++m_offset;
	}


	// buffer中剩余的字节数 不能小于从头文件解析出来的字节数
	if (buf_size < size) {
		return 1;
	}

	memcpy(data, buffer, size);
	*data_size = size;
	return 0;
}


/*
每次windows sdk会调用这个函数取出相应字节数的pcm 数据
所以需要每次准备好适量的数据然后返回
*/
int AACReader::LoadData(const int frames, const BYTE* data, int* bDone) {
	int byte_count = m_channels * (m_bit_depth / 8) * frames;
	HRESULT notImpl = 0;

	if (m_exceed > 0 && !m_file.eof()) {
		memcpy((void*)data, m_exceed_buf, m_exceed);
		m_pcm_ptr_pos = m_exceed;
		m_exceed = 0;
	}

	// 如果数据够一帧则继续解码
	if (m_aac_size > 0) {
#ifdef _DEBUG
		printf("[DEBUG]m_cur_pos:%d\n", m_pcm_ptr_pos);
		printf("[DEBUG]m_aac_size:%d\n", m_aac_size);
		printf("[DEBUG]m_read_byte:%d\n", m_read_byte);
#endif // _DEBUG

		goto DECODE;
	}


	// 读取文件 然后判断是否至少可以得到一帧adts 如果得不到则继续读取 直到通过为止
	// 解码 结束 剩余的字节应该在继续保存并放入到下次 循环当中
READ:
	if (m_read_byte <= 0 && m_file.eof()) {
		*bDone = TRUE;
		data = NULL;
		return notImpl;
	}

	m_file.read((char*)m_aac_buf, BUFFER_MAX_LEN);
	m_read_byte = m_file.gcount();



	if (m_temp_ptr == nullptr) {
		m_temp_ptr = m_aac_buf;
	}

	if (m_aac_size > 0 && m_temp_ptr != nullptr) {

		memcpy(m_temp_file_buf + ((size_t)m_aac_size - (size_t)m_offset), m_aac_buf, m_read_byte);

		m_temp_ptr = m_temp_file_buf;
		
		m_read_byte += (m_aac_size - m_offset);
		
	}
	
	if (m_first && get_one_ADTS_frame(m_temp_ptr, m_read_byte, m_frame, &m_size) == 0) {
		unsigned long samplerate;
		unsigned char channels;
		NeAACDecInit(m_decoder, m_frame, m_size, &samplerate, &channels);
		m_first = FALSE;
	}
	

DECODE:
	int pcm_size = 0;
	while (1) {
		
		int adts_ret = get_one_ADTS_frame(m_temp_ptr, m_read_byte, m_frame, &m_size);
		if (adts_ret == -1) {
			break;
		}
		else if (adts_ret == 1 ) {
			// buffer不足 需要补充
			if (m_file.eof()) {
				*bDone = TRUE;
				data = NULL;
				return notImpl;
			}
			else {
				m_aac_size = m_read_byte;
				memcpy(m_temp_file_buf, m_temp_ptr + m_offset, (size_t)m_aac_size - (size_t)m_offset);
				goto READ;
			}
		}
		
		m_pcm_buf = (BYTE*)NeAACDecDecode(m_decoder, &m_info, m_frame, m_size);
		

		if (m_info.error != 0) {
			printf("[ERROR] Decode Failed!\n");
			exit(-1);
		}
		
		m_temp_ptr += m_size;
		m_read_byte -= m_size;
		// 存下每次解码后的buffer 和 剩余字节 
		m_aac_size = m_read_byte;

		// 解码成功
		if ((m_info.error == 0) && (m_info.samples > 0)) {
			//printf("decoding .... success\n"); 4096
			pcm_size = m_info.samples * m_info.channels;
			// 判断是否超过了要返回的字节数
			m_exceed = m_pcm_ptr_pos + pcm_size - byte_count;
			if (m_exceed > 0) {
				// 还可以装多少字节
				int remain = pcm_size - m_exceed;
				memcpy((void*)(data + m_pcm_ptr_pos), m_pcm_buf, remain);
				m_pcm_ptr_pos += remain;
				// 剩下超出的字节 存到别的地方 下次利用
				memcpy(m_exceed_buf, m_pcm_buf + remain, m_exceed);
				break;
			}
			else {
				memcpy((void*)(data + m_pcm_ptr_pos), m_pcm_buf, pcm_size);
				m_pcm_ptr_pos += pcm_size;
				// 如果正好等于 则跳出
				if (m_pcm_ptr_pos == byte_count) {
					break;
				}
			}
		}
	}

#ifdef _DEBUG
		// 将指针重置 但超出的exceed需要保留重新赋值
	printf("[DEBUG]m_aac_size:%d\n", m_aac_size);
	printf("[DEBUG]m_read_byte:%d\n", m_read_byte);
	printf("[DEBUG]m_cur_pos:%d\n", m_pcm_ptr_pos);
#endif // _DEBUG

	m_pcm_ptr_pos = 0;
	return notImpl;
}