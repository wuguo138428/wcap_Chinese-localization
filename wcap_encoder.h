#include "wcap.h"

#include <d3d11.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#define ENCODER_VIDEO_BUFFER_COUNT 8
#define ENCODER_AUDIO_BUFFER_COUNT 16

typedef struct Encoder {
	DWORD InputWidth;   // width to what input will be cropped
	DWORD InputHeight;  // height to what input will be cropped
	DWORD OutputWidth;  // width of video output
	DWORD OutputHeight; // height of video output
	DWORD FramerateNum; // video output framerate numerator
	DWORD FramerateDen; // video output framerate denumerator
	UINT64 StartTime;   // time in QPC ticks since first call of NewFrame

	IMFAsyncCallback VideoSampleCallback;
	IMFAsyncCallback AudioSampleCallback;
	IMFDXGIDeviceManager* Manager;
	ID3D11Device* Device;
	ID3D11DeviceContext* Context;
	IMFSinkWriter* Writer;
	int VideoStreamIndex;
	int AudioStreamIndex;

	ID3D11Texture2D*        Texture;
	ID3D11RenderTargetView* TextureView[ENCODER_VIDEO_BUFFER_COUNT];
	IMFSample*              VideoSample[ENCODER_VIDEO_BUFFER_COUNT];
	IMFTrackedSample*       VideoTracked[ENCODER_VIDEO_BUFFER_COUNT];

	DWORD         VideoIndex; // next index to use
	volatile LONG VideoCount; // how many samples are currently available to use

	IMFTransform* Resampler;
	IMFSample* AudioSample[ENCODER_AUDIO_BUFFER_COUNT];
	IMFTrackedSample* AudioTracked[ENCODER_AUDIO_BUFFER_COUNT];
	IMFSample* AudioInputSample;
	IMFMediaBuffer* AudioInputBuffer;
	HANDLE AudioSemaphore;
	DWORD AudioFrameSize;
	DWORD AudioSampleRate;
	DWORD AudioIndex; // next index to use

} Encoder;

typedef struct {
	BOOL FragmentedOutput;
	BOOL HardwareEncoder;
	DWORD Width;
	DWORD Height;
	DWORD MaxWidth;
	DWORD MaxHeight;
	DWORD FramerateNum;
	DWORD FramerateDen;
	DWORD VideoBitrate;

	WAVEFORMATEX* AudioFormat;
	DWORD AudioBitrate;

} EncoderConfig;

void Encoder_Init(Encoder* Encoder, ID3D11Device* Device, ID3D11DeviceContext* Context);
BOOL Encoder_Start(Encoder* Encoder, LPWSTR FileName, const EncoderConfig* Config);
void Encoder_Stop(Encoder* Encoder);

BOOL Encoder_NewFrame(Encoder* Encoder, ID3D11Texture2D* Texture, RECT Rect, UINT64 Time, UINT64 TimePeriod);
void Encoder_NewSamples(Encoder* Encoder, LPCVOID Samples, DWORD VideoCount, UINT64 Time, UINT64 TimePeriod);
void Encoder_GetStats(Encoder* Encoder, DWORD* Bitrate, DWORD* LengthMsec, UINT64* FileSize);
