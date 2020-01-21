//-----------------------------------------------------------
// Record an audio stream from the default audio capture
// device. The RecordAudioStream function allocates a shared
// buffer big enough to hold one second of PCM audio data.
// The function uses this buffer to stream data from the
// capture device. The main loop runs every 1/2 second.
//-----------------------------------------------------------

#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <audiopolicy.h>
#include "AudioListener.h"
#include <mmreg.h>
#include <stdio.h>
#include <iostream>

AudioListener::AudioListener(int BitsPerSample, int FormatTag, int BlockAlign, int XSize, int device)
{
	// nBlockAlign必须等于nChannels和wBitsPerSample的乘积除以8（每字节位数）
	HRESULT hr;
	REFERENCE_TIME hnsRequestedDuration = m_refTimesPerSec;

	hr = CoCreateInstance(
		m_CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, m_IID_IMMDeviceEnumerator,
		(void**)&m_pEnumerator);

	if (hr)	throw hr;
	if (device == 0) {
		hr = m_pEnumerator->GetDefaultAudioEndpoint(
			eCapture, eConsole, &m_pDevice); //set this to capture from default recording device instead of render device
	}
	else {
		hr = m_pEnumerator->GetDefaultAudioEndpoint(
			eRender, eConsole, & m_pDevice);
	}
	
	if (hr)	throw hr;

	hr = m_pDevice->Activate(
		m_IID_IAudioClient, CLSCTX_ALL,
		NULL, (void**)&m_pAudioClient);
	if (hr)	throw hr;

	hr = m_pAudioClient->GetMixFormat(&m_pwfx);
	if (hr)	throw hr;

	m_pwfx->wBitsPerSample = BitsPerSample;
	m_pwfx->nBlockAlign = BlockAlign;
	m_pwfx->wFormatTag = FormatTag;
	m_pwfx->cbSize = XSize;
	m_pwfx->nAvgBytesPerSec = m_pwfx->nSamplesPerSec * m_pwfx->nBlockAlign;

	std::cout << "[INFO]capture formats:" << std::endl;
	std::cout << m_pwfx->nChannels << std::endl;
	std::cout << m_pwfx->nSamplesPerSec << std::endl;
	std::cout << m_pwfx->wBitsPerSample << std::endl;
	std::cout << m_pwfx->nAvgBytesPerSec << std::endl;
	std::cout << "****************" << std::endl;

	if (device == 0) {
		hr = m_pAudioClient->Initialize(
			//AUDCLNT_SHAREMODE_EXCLUSIVE,
			AUDCLNT_SHAREMODE_SHARED,
			0, //set this instead of loopback to capture from default recording device
			hnsRequestedDuration,
			0,
			m_pwfx,
			NULL);
	}
	else {
		hr = m_pAudioClient->Initialize(
			//AUDCLNT_SHAREMODE_EXCLUSIVE,
			AUDCLNT_SHAREMODE_SHARED,
			AUDCLNT_STREAMFLAGS_LOOPBACK,
			hnsRequestedDuration,
			0,
			m_pwfx,
			NULL);
	}
	
	if (hr)	throw hr;

	// 这个buffersize，指的是缓冲区最多可以存放多少帧的数据量
	hr = m_pAudioClient->GetBufferSize(&m_bufferFrameCount);


	if (hr)	throw hr;

	hr = m_pAudioClient->GetService(
		m_IID_IAudioCaptureClient,
		(void**)&m_pCaptureClient);
	if (hr) throw hr;


	// Calculate the actual duration of the allocated buffer.
	m_hnsActualDuration = (double)m_refTimesPerSec *
		m_bufferFrameCount / m_pwfx->nSamplesPerSec;

}
AudioListener::~AudioListener()
{
	CoTaskMemFree(m_pwfx);
	SAFE_RELEASE(m_pEnumerator)
	SAFE_RELEASE(m_pDevice)
	SAFE_RELEASE(m_pAudioClient)
	SAFE_RELEASE(m_pCaptureClient)
}
HRESULT AudioListener::RecordAudioStream(IAudioSink* Sink)
{
	HRESULT hr;
	BOOL bDone = FALSE;
	BYTE *pData;
	DWORD flags;
	UINT32 packetLength = 0;
	UINT32 numFramesAvailable;

	hr = m_pAudioClient->Start();  // Start recording.
	if (hr) throw hr;


	// Each loop f6ills about half of the shared buffer.
	while (bDone == FALSE)
	{
		// 让程序暂停运行一段时间，缓冲区里在这段时间会被填充数据
		Sleep(m_hnsActualDuration / m_refTimesPerMS /2);
		
		hr = m_pCaptureClient->GetNextPacketSize(&packetLength);
		if (hr) throw hr;

		while (packetLength != 0)
		{
			// 锁定缓冲区，获取数据 
			hr = m_pCaptureClient->GetBuffer(
				&pData,
				&numFramesAvailable,
				&flags, NULL, NULL);
			if (hr) throw hr;
			
			if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
			{
				pData = NULL;  // Tell CopyData to write silence.
			}
			
			// Copy the available capture data to the audio sink.
			std::cout << "[INFO] Recording..." << std::endl;
			hr = Sink->CopyData(
				pData, numFramesAvailable, &bDone);
			
			if (hr) throw hr;
			
			hr = m_pCaptureClient->ReleaseBuffer(numFramesAvailable);
			if (hr) throw hr;

			hr = m_pCaptureClient->GetNextPacketSize(&packetLength);
			if (hr) throw hr;
		}
	}

	std::cout << "[INFO] Recoding Finished!" << std::endl;
	hr = m_pAudioClient->Stop();  // Stop recording.
	if (hr) throw hr;
	return hr;
}