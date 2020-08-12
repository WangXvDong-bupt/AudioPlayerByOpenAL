#include "AudioPlayer.h"

#define NUMBUFFERS (12)
#define SERVICE_UPDATE_PERIOD (20)
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
AudioPlayer::AudioPlayer()
{

}


AudioPlayer::~AudioPlayer()
{
	//SDL_CloseAudio();//关闭音频设备 
	swr_free(&swrCtx);
}


void AudioPlayer::audioSetting()
{
	//重采样设置选项-----------------------------------------------------------start
	//输入的采样格式
	in_sample_fmt = pCodeCtx->sample_fmt;
	//输出的采样格式 16bit PCM
	out_sample_fmt = AV_SAMPLE_FMT_S16;
	//输入的采样率
	in_sample_rate = pCodeCtx->sample_rate;
	//输出的采样率
	out_sample_rate = pCodeCtx->sample_rate;
	//输入的声道布局
	in_ch_layout = pCodeCtx->channel_layout;
	if (in_ch_layout <= 0)
	{
		in_ch_layout = av_get_default_channel_layout(pCodeCtx->channels);
	}
	//输出的声道布局
	out_ch_layout = AV_CH_LAYOUT_STEREO;

	//frame->16bit 44100 PCM 统一音频采样格式与采样率
	swrCtx = swr_alloc();
	swr_alloc_set_opts(swrCtx, out_ch_layout, out_sample_fmt, out_sample_rate, in_ch_layout, in_sample_fmt,
		in_sample_rate, 0, NULL);
	swr_init(swrCtx);
	//重采样设置选项-----------------------------------------------------------end

	//获取输出的声道个数
	out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);
}


int AudioPlayer::setAudioSDL()
{
	//获取输出的声道个数
	out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);
	//SDL_AudioSpec
	wanted_spec.freq = out_sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = out_channel_nb;
	wanted_spec.silence = 0;
	wanted_spec.samples = pCodeCtx->frame_size;
	wanted_spec.callback = fill_audio;//回调函数
	wanted_spec.userdata = pCodeCtx;
	if (SDL_OpenAudio(&wanted_spec, NULL) < 0) {
		printf("can't open audio.\n");
		return -1;
	}
}

static Uint8* audio_chunk;
//设置音频数据长度
static Uint32 audio_len;
static Uint8* audio_pos;

void  AudioPlayer::fill_audio(void* udata, Uint8* stream, int len) {
	//SDL 2.0
	SDL_memset(stream, 0, len);
	if (audio_len == 0)		//有数据才播放
		return;
	len = (len > audio_len ? audio_len : len);

	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
}

void AudioPlayer::feedAudioData(ALuint source, ALuint alBufferId, int out_sample_rate, FILE* fpPCM_open)
{
	int ret = 0;
	int ndatasize = 4096;
	char ndata[4096 + 1] = { 0 };
	//fseek(fpPCM_open, 4096, SEEK_CUR);
	ret = fread(ndata, 1, ndatasize, fpPCM_open);
	alBufferData(alBufferId, AL_FORMAT_STEREO16, ndata, ndatasize, out_sample_rate);
	alSourceQueueBuffers(source, 1, &alBufferId);
}



int AudioPlayer::playByOpenAL()
{
	//--------初始化openAL--------
	ALuint source;
	ALCdevice* pDevice = alcOpenDevice(NULL);
	ALCcontext* pContext = alcCreateContext(pDevice, NULL);
	alcMakeContextCurrent(pContext);
	if (alcGetError(pDevice) != ALC_NO_ERROR)
		return AL_FALSE;
	
	alGenSources(1, &source);
	if (alGetError() != AL_NO_ERROR)
	{
		printf("Couldn't generate audio source\n");
		return -1;
	}
	ALfloat SourcePos[] = { 0.0,0.0,0.0 };
	ALfloat SourceVel[] = { 0.0,0.0,0.0 };
	ALfloat ListenerPos[] = { 0.0,0.0 };
	ALfloat ListenerVel[] = { 0.0,0.0,0.0 };
	ALfloat ListenerOri[] = { 0.0,0.0,-1.0,0.0,1.0,0.0 };
	alSourcef(source, AL_PITCH, 1.0);
	alSourcef(source, AL_GAIN, 1.0);
	alSourcefv(source, AL_POSITION, SourcePos);
	alSourcefv(source, AL_VELOCITY, SourceVel);
	alSourcef(source, AL_REFERENCE_DISTANCE, 50.0f);
	alSourcei(source, AL_LOOPING, AL_FALSE);
	//--------初始化openAL--------
	ALuint alBufferArray[NUMBUFFERS];
	alGenBuffers(NUMBUFFERS, alBufferArray);



	//编码数据
	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
	//av_init_packet(packet);		//初始化
	//解压缩数据
	AVFrame* frame = av_frame_alloc();
	//存储pcm数据
	uint8_t* out_buffer = (uint8_t*)av_malloc(2 * out_sample_rate);

	int ret, got_frame, framecount = 0;
	int out_buffer_size;

	//打开写入PCM文件
	FILE* fPCM = fopen("output.pcm", "wb");

	while (av_read_frame(pFormatCtx, packet) >= 0)
	{
		if (packet->stream_index == index)
		{
			//解码AVPacket->AVFrame
			ret = avcodec_decode_audio4(pCodeCtx, frame, &got_frame, packet);

			/*ret = avcodec_send_packet(pCodeCtx, packet);
			if (ret != 0)
			{
				printf("%s/n", "error");
				exit(1);
			}
			got_frame = avcodec_receive_frame(pCodeCtx, frame);			//output=0》success, a frame was returned
			while ((got_frame = avcodec_receive_frame(pCodeCtx, frame)) == 0) {
					//读取到一帧音频或者视频
					//处理解码后音视频 frame
					swr_convert(swrCtx, &out_buffer, 2 * 44100, (const uint8_t**)frame->data, frame->nb_samples);
			}*/
			if (got_frame)
			{
				swr_convert(swrCtx, &out_buffer, 2 * out_sample_rate, (const uint8_t**)frame->data, frame->nb_samples);
				out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb, frame->nb_samples, out_sample_fmt, 1);
				fwrite(out_buffer, 1, out_buffer_size, fPCM);
			}

		}
		av_packet_unref(packet);
	}
	FILE* fpPCM_open = NULL;
	if ((fpPCM_open = fopen("output.pcm", "rb")) == NULL)
	{
		printf("Failed open the PCM file\n");
		return -1;
	}
	

	fseek(fpPCM_open, 0, SEEK_SET);
	for (int i = 0; i < NUMBUFFERS; i++)
	{
		feedAudioData(source, alBufferArray[i], out_sample_rate, fpPCM_open);
	}

	//开始播放
	alSourcePlay(source);
	ALint iTotalBuffersProcessed = 0;
	ALint iBuffersProcessed;
	ALint iState;
	ALuint bufferId;
	ALint iQueuedBuffers;
	while (true)
	{
		iBuffersProcessed = 0;
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &iBuffersProcessed);
		iTotalBuffersProcessed += iBuffersProcessed;
		//iTotalBuffersProcessed += iBuffersProcessed;
		while (iBuffersProcessed > 0) {
			bufferId = 0;
			alSourceUnqueueBuffers(source, 1, &bufferId);
			feedAudioData(source, bufferId, out_sample_rate, fpPCM_open);
			iBuffersProcessed -= 1;
		}
		alGetSourcei(source, AL_SOURCE_STATE, &iState);
		if (iState != AL_PLAYING) {
			alGetSourcei(source, AL_BUFFERS_QUEUED, &iQueuedBuffers);
			if (iQueuedBuffers) {
				alSourcePlay(source);
			}
			else {
				//停止播放
				break;
			}
		}
	}
	alSourceStop(source);
	alSourcei(source, AL_BUFFER, 0);
	alDeleteSources(1, &source);
	alDeleteBuffers(NUMBUFFERS, alBufferArray);

	fclose(fpPCM_open);
	fclose(fPCM);

	alcMakeContextCurrent(NULL);
	alcDestroyContext(pContext);
	pContext = NULL;
	alcCloseDevice(pDevice);
	pDevice = NULL;

	av_frame_free(&frame);
	av_free(out_buffer);

	return 0;
}


