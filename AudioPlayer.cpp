#include "AudioPlayer.h"

#define RATE 48000
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
	//SDL_CloseAudio();//�ر���Ƶ�豸 
	swr_free(&swrCtx);
}


void AudioPlayer::audioSetting()
{
	//�ز�������ѡ��-----------------------------------------------------------start
	//����Ĳ�����ʽ
	in_sample_fmt = pCodeCtx->sample_fmt;
	//����Ĳ�����ʽ 16bit PCM
	out_sample_fmt = AV_SAMPLE_FMT_S16;
	//����Ĳ�����
	in_sample_rate = pCodeCtx->sample_rate;
	//����Ĳ�����
	out_sample_rate = RATE;
	//�������������
	in_ch_layout = pCodeCtx->channel_layout;
	if (in_ch_layout <= 0)
	{
		in_ch_layout = av_get_default_channel_layout(pCodeCtx->channels);
	}
	//�������������
	out_ch_layout = AV_CH_LAYOUT_STEREO;

	//frame->16bit 44100 PCM ͳһ��Ƶ������ʽ�������
	swrCtx = swr_alloc();
	swr_alloc_set_opts(swrCtx, out_ch_layout, out_sample_fmt, out_sample_rate, in_ch_layout, in_sample_fmt,
		in_sample_rate, 0, NULL);
	swr_init(swrCtx);
	//�ز�������ѡ��-----------------------------------------------------------end

	//��ȡ�������������
	out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);
}


int AudioPlayer::setAudioSDL()
{
	//��ȡ�������������
	out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);
	//SDL_AudioSpec
	wanted_spec.freq = out_sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = out_channel_nb;
	wanted_spec.silence = 0;
	wanted_spec.samples = pCodeCtx->frame_size;
	wanted_spec.callback = fill_audio;//�ص�����
	wanted_spec.userdata = pCodeCtx;
	if (SDL_OpenAudio(&wanted_spec, NULL) < 0) {
		printf("can't open audio.\n");
		return -1;
	}
}

static Uint8* audio_chunk;
//������Ƶ���ݳ���
static Uint32 audio_len;
static Uint8* audio_pos;

void  AudioPlayer::fill_audio(void* udata, Uint8* stream, int len) {
	//SDL 2.0
	SDL_memset(stream, 0, len);
	if (audio_len == 0)		//�����ݲŲ���
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
	//--------��ʼ��openAL--------
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
	//--------��ʼ��openAL--------
	ALuint alBufferArray[NUMBUFFERS];
	alGenBuffers(NUMBUFFERS, alBufferArray);



	//��������
	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
	//av_init_packet(packet);		//��ʼ��
	//��ѹ������
	AVFrame* frame = av_frame_alloc();
	//�洢pcm����
	uint8_t* out_buffer = (uint8_t*)av_malloc(2 * RATE);

	int ret, got_frame, framecount = 0;
	int out_buffer_size;

	//��д��PCM�ļ�
	FILE* fPCM = fopen("output.pcm", "wb");

	while (av_read_frame(pFormatCtx, packet) >= 0)
	{
		if (packet->stream_index == index)
		{
			//����AVPacket->AVFrame
			ret = avcodec_decode_audio4(pCodeCtx, frame, &got_frame, packet);

			/*ret = avcodec_send_packet(pCodeCtx, packet);
			if (ret != 0)
			{
				printf("%s/n", "error");
				exit(1);
			}
			got_frame = avcodec_receive_frame(pCodeCtx, frame);			//output=0��success, a frame was returned
			while ((got_frame = avcodec_receive_frame(pCodeCtx, frame)) == 0) {
					//��ȡ��һ֡��Ƶ������Ƶ
					//������������Ƶ frame
					swr_convert(swrCtx, &out_buffer, 2 * 44100, (const uint8_t**)frame->data, frame->nb_samples);
			}*/
			if (got_frame)
			{
				swr_convert(swrCtx, &out_buffer, 2 * 44100, (const uint8_t**)frame->data, frame->nb_samples);
				out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb, frame->nb_samples, out_sample_fmt, 1);
				fwrite(out_buffer, 1, out_buffer_size, fPCM);
			}

			/*if (ret < 0) {
				printf("%s", "�������");
			}

			//��0�����ڽ���

			if (got_frame)
			{
				//printf("����%d֡", framecount++);
				swr_convert(swrCtx, &out_buffer, 2 * 44100, (const uint8_t**)frame->data, frame->nb_samples);
				//��ȡsample��size
				out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb, frame->nb_samples,
					out_sample_fmt, 1);
				//������Ƶ���ݻ���,PCM����
				audio_chunk = (Uint8*)out_buffer;
				//������Ƶ���ݳ���
				audio_len = out_buffer_size;
				audio_pos = audio_chunk;
				//�ط���Ƶ����
				SDL_PauseAudio(0);
				while (audio_len > 0)//�ȴ�ֱ����Ƶ���ݲ������!
					SDL_Delay(10);
				packet->data += ret;
				packet->size -= ret;
			}*/
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

	//��ʼ����
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
				//ֹͣ����
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



int AudioPlayer::play()
{
	//��������
	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
	//av_init_packet(packet);		//��ʼ��
	//��ѹ������
	AVFrame* frame = av_frame_alloc();
	//�洢pcm����
	uint8_t* out_buffer = (uint8_t*)av_malloc(2 * RATE);

	int ret, got_frame, framecount = 0;
	//һ֡һ֡��ȡѹ������Ƶ����AVPacket

	FILE* fPCM = fopen("output2.pcm", "wb");

	while (av_read_frame(pFormatCtx, packet) >= 0) {
		if (packet->stream_index == index) {
				//����AVPacket->AVFrame
			ret = avcodec_decode_audio4(pCodeCtx, frame, &got_frame, packet);

			//ret = avcodec_send_packet(pCodeCtx, packet);
			//if (ret != 0) { printf("%s/n", "error"); }
			//got_frame = avcodec_receive_frame(pCodeCtx, frame);			//output=0��success, a frame was returned
			//while ((got_frame = avcodec_receive_frame(pCodeCtx, frame)) == 0) {
			//		//��ȡ��һ֡��Ƶ������Ƶ
			//		//������������Ƶ frame
			//		swr_convert(swrCtx, &out_buffer, 2 * 44100, (const uint8_t**)frame->data, frame->nb_samples);
			//}


			if (ret < 0) {
				 printf("%s", "�������");
			}

			//��0�����ڽ���
			int out_buffer_size;
			if (got_frame) {
				//printf("����%d֡", framecount++);
				swr_convert(swrCtx, &out_buffer, 2 * 44100, (const uint8_t**)frame->data, frame->nb_samples);
				//��ȡsample��size
				out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb, frame->nb_samples,
					out_sample_fmt, 1);

				fwrite(out_buffer, 1, out_buffer_size, fPCM);

				//������Ƶ���ݻ���,PCM����
				audio_chunk = (Uint8*)out_buffer;
				//������Ƶ���ݳ���
				audio_len = out_buffer_size;
				audio_pos = audio_chunk;
				//�ط���Ƶ����
				SDL_PauseAudio(0);
				while (audio_len > 0)//�ȴ�ֱ����Ƶ���ݲ������!
					SDL_Delay(10);
				packet->data += ret;
				packet->size -= ret;
			}
		}
		//av_packet_unref(packet);
	}

	fclose(fPCM);

	av_free(out_buffer);
	av_frame_free(&frame);
	//av_free_packet(packet);
	av_packet_unref(packet);
	SDL_CloseAudio();//�ر���Ƶ�豸
	swr_free(&swrCtx);
	return 0;
}

int decode(uint8_t* buf, int bufSize, AVPacket* packet, AVCodecContext* codecContext,
	SwrContext* swr, int dstRate, int dstNbChannels, enum AVSampleFormat* dstSampleFmt) {


	unsigned int bufIndex = 0;
	unsigned int dataSize = 0;
	int ret = 0;
	AVPacket* pkt = av_packet_alloc();
	AVFrame* frame = av_frame_alloc();
	if (!frame) {
		cout << "Error allocating the frame" << endl;
		av_packet_unref(packet);
		return 0;
	}
	while (packet->size > 0) {
		int gotFrame = 0;
		int result = avcodec_decode_audio4(codecContext, frame, &gotFrame, packet);
		if (result >= 0 && gotFrame)
		{
			packet->size -= result;
			packet->data += result;
			int dstNbSamples = av_rescale_rnd(frame->nb_samples, dstRate, codecContext->sample_rate, AV_ROUND_UP);
			uint8_t** dstData = NULL;
			int dstLineSize;
			if (av_samples_alloc_array_and_samples(&dstData, &dstLineSize, dstNbChannels, dstNbSamples, *dstSampleFmt, 0) < 0) {
				cerr << "Could not allocate destination samples" << endl;
				dataSize = 0;
				break;
			}
			int ret = swr_convert(swr, dstData, dstNbSamples, (const uint8_t**)frame->extended_data, frame->nb_samples);
			if (ret < 0) {
				cerr << "Error while converting" << endl;
				dataSize = 0;
				break;
			}
			int dstBufSize = av_samples_get_buffer_size(&dstLineSize, dstNbChannels, ret, *dstSampleFmt, 1);
			if (dstBufSize < 0) {
				cerr << "Error av_samples_get_buffer_size()" << endl;
				dataSize = 0;
				break;
			}
			if (dataSize + dstBufSize > bufSize) {
				cerr << "dataSize + dstBufSize > bufSize" << endl;
				dataSize = 0;
				break;
			}

			memcpy((uint8_t*)buf + bufIndex, dstData[0], dstBufSize);
			bufIndex += dstBufSize;
			dataSize += dstBufSize;
			if (dstData)
				av_freep(&dstData[0]);
			av_freep(&dstData);
		}
		else {
			packet->size = 0;
			packet->data = NULL;
		}
	}
	av_packet_unref(packet);
	av_free(frame);
	return dataSize;

}