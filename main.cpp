#include "AudioListener.h"
#include "IAudioSink.h"
#include "WriteAac.h"
#include "FileWriteSink.h"
#include "AudioRender.h"
#include "ReadPcm.h"
#include "ReadAcc.h"


int main()
{
	CoInitialize(nullptr);
	int bit_depth = 16;
	int channels = 2;
	int bit_rate = 64000;
	int sample_rate = 44100;

	const char* aac_path = "test.aac";
	const char* pcm_path = "test.pcm";

	int capture_method = 0;  // 1: capture from speaker  0: capture from micphone
	// player

	//AudioRender player(bit_depth, WAVE_FORMAT_PCM, 4, 0);
	////PCMReader sink(channels, bitDepth, pcm_path);
	//AACReader sink(channels, bit_depth, sample_rate, aac_path);
	//player.PlayAudioStream(&sink);


	// capturer

	AudioListener listener(bit_depth, WAVE_FORMAT_PCM, 4, 0, capture_method);
	//FileWriteSink sink(channels, bit_depth, pcm_path);
	AACWriter sink(channels, bit_depth, sample_rate, bit_rate, aac_path);
	listener.RecordAudioStream(&sink);
}