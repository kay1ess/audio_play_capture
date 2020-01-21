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
#include <mmreg.h>
#include <stdio.h>
#include <iostream>
#include "AudioRender.h"


AudioRender::AudioRender(int BitsPerSample, int FormatTag, int BlockAlign, int XSize)
{
	// nBlockAlign�������nChannels��wBitsPerSample�ĳ˻�����8��ÿ�ֽ�λ����
	HRESULT hr;
	REFERENCE_TIME hnsRequestedDuration = m_refTimesPerSec;

	hr = CoCreateInstance(
		m_CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, m_IID_IMMDeviceEnumerator,
		(void**)&m_pEnumerator);
	if (hr)	throw hr;

	// ��ȡ��Ĭ�ϵĲ����豸
	hr = m_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_pDevice);
	if (hr)	throw hr;

	// �����豸
	hr = m_pDevice->Activate(
		m_IID_IAudioClient, CLSCTX_ALL,
		NULL, (void**)&m_pAudioClient);
	if (hr)	throw hr;

	// ��������ʽ
	hr = m_pAudioClient->GetMixFormat(&m_pwfx);
	if (hr)	throw hr;

	// ������Ƶ��ʽ
	m_pwfx->wBitsPerSample = BitsPerSample;
	m_pwfx->nBlockAlign = BlockAlign;
	m_pwfx->wFormatTag = FormatTag;
	m_pwfx->cbSize = XSize;
	m_pwfx->nAvgBytesPerSec = m_pwfx->nSamplesPerSec * m_pwfx->nBlockAlign;


	// ��ʾ������Ϣ
	std::cout << "***********************************" << std::endl;
	std::cout << "AUDIO FORMAT:" << std::endl;
	std::cout << m_pwfx->nChannels << std::endl;
	std::cout << m_pwfx->nSamplesPerSec << std::endl;
	std::cout << m_pwfx->wBitsPerSample << std::endl;
	std::cout << m_pwfx->nAvgBytesPerSec << std::endl;
	std::cout << "***********************************" << std::endl;
	
	// ��ʼ��������ʼ����Ƶ����
	hr = m_pAudioClient->Initialize(
		//AUDCLNT_SHAREMODE_EXCLUSIVE,
		AUDCLNT_SHAREMODE_SHARED,
		0, //set this instead of loopback to capture from default recording device
		hnsRequestedDuration,
		0,
		m_pwfx,
		NULL);
	if (hr)	throw hr;

	// ���buffersize��ָ���ǻ����������Դ�Ŷ���֡��������
	hr = m_pAudioClient->GetBufferSize(&m_bufferFrameCount);


	if (hr)	throw hr;

	hr = m_pAudioClient->GetService(
		m_IID_IAudioRenderClient,
		(void**)&m_pRenderClient);
	if (hr) throw hr;


	// Calculate the actual duration of the allocated buffer.
	m_hnsActualDuration = (double)m_refTimesPerSec *
		m_bufferFrameCount / m_pwfx->nSamplesPerSec;

}

AudioRender::~AudioRender()
{
	CoTaskMemFree(m_pwfx);
	SAFE_RELEASE(m_pEnumerator)
	SAFE_RELEASE(m_pDevice)
	SAFE_RELEASE(m_pAudioClient)
	SAFE_RELEASE(m_pRenderClient)
}

HRESULT AudioRender::PlayAudioStream(IAudioSink* Sink)
{
	HRESULT hr;
	BOOL bDone = FALSE;
	BYTE* pData;
	DWORD flags = 0;
	UINT32 packetLength = 0;
	UINT32 numFramesAvailable;

	// ��ʼ������
	hr = m_pRenderClient->GetBuffer(m_bufferFrameCount, &pData);
	if (hr) throw hr;

	hr = Sink->LoadData(m_bufferFrameCount, pData, &bDone);
	if (hr) throw hr;

	hr = m_pRenderClient->ReleaseBuffer(m_bufferFrameCount, flags);
	

	hr = m_pAudioClient->Start();  // start playing
	if (hr) throw hr;

	// Each loop f6ills about half of the shared buffer.
	while (bDone == FALSE)
	{
		// �ó�����ͣ����һ��ʱ�䣬�������������ʱ��ᱻ�������
		Sleep(m_hnsActualDuration / m_refTimesPerMS / 2);

		hr = m_pAudioClient->GetCurrentPadding(&packetLength);
		if (hr) throw hr;

		numFramesAvailable = m_bufferFrameCount - packetLength;

		// ��������������ȡ���� 
		hr = m_pRenderClient->GetBuffer(
			numFramesAvailable, &pData);
		std::cout << "[INFO] Playing(audio frames):" << numFramesAvailable << std::endl;

		if (hr) throw hr;

		hr = Sink->LoadData(
			numFramesAvailable, pData, &bDone);
		if (hr) throw hr;

		hr = m_pRenderClient->ReleaseBuffer(numFramesAvailable, flags);
		if (hr) throw hr;
	}

	hr = m_pAudioClient->Stop();  // Stop playing
	std::cout << "[INFO] Playing Finished:"  << std::endl;
	if (hr) throw hr;
	return hr;
}