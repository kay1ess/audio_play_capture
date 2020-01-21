#pragma once
typedef unsigned char BYTE;
class IAudioSink
{
public:
	virtual ~IAudioSink() {}
	virtual int CopyData(const BYTE* data, const int frames, int* done) { return 0; };
	virtual int LoadData(const int frames, const BYTE* data, int* done) { return 0; };
};