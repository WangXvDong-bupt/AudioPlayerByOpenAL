#pragma once
#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H
#include "Player.h"
#include "OpenAL/include/al.h"
#include "OpenAL/include/alc.h"
#include <chrono>
#include <thread>
#include <string>
#include <iomanip>
#include<iostream>

class AudioPlayer :public Player
{
public:
	AudioPlayer();
	~AudioPlayer();
private:
	SwrContext* swrCtx;

	//重采样设置选项-----------------------------------------------------------start

	//输入的采样格式
	enum AVSampleFormat in_sample_fmt;
	//输出的采样格式 16bit PCM
	enum AVSampleFormat out_sample_fmt;
	//输入的采样率
	int in_sample_rate;
	//输出的采样率
	int out_sample_rate;
	//输入的声道布局
	uint64_t in_ch_layout;
	//输出的声道布局
	uint64_t out_ch_layout;
	//重采样设置选项-----------------------------------------------------------end

	//SDL_AudioSpec
	SDL_AudioSpec wanted_spec;
	//获取输出的声道个数
	int out_channel_nb;

public:
	void audioSetting();
	int AudioPlayer::setAudioSDL();
	int play();
	int playByOpenAL();
private:
	static void fill_audio(void* udata, Uint8* stream, int len);
	void AudioPlayer::feedAudioData(ALuint source, ALuint alBufferId, int out_sample_rate, FILE* fpPCM_open);

	typedef struct PacketQueue {
		AVPacketList* pFirstPkt;
		AVPacketList* pLastPkt;
		int numOfPackets;
	} PacketQueue;
	void initPacketQueue(PacketQueue* q) {
		memset(q, 0, sizeof(PacketQueue));
	}
	int pushPacketToPacketQueue(PacketQueue* pPktQ, AVPacket* pPkt) {
		AVPacketList* pPktList;
		//if (av_dup_packet(pPkt) < 0) {
		if (pPkt->size < 0) {
			return -1;
		}
		pPktList = (AVPacketList*)av_malloc(sizeof(AVPacketList));
		if (!pPktList) {
			return -1;
		}
		pPktList->pkt = *pPkt;
		pPktList->next = NULL;
		if (!pPktQ->pLastPkt) {
			pPktQ->pFirstPkt = pPktList;
		}
		else {
			pPktQ->pLastPkt->next = pPktList;
		}
		pPktQ->pLastPkt = pPktList;
		pPktQ->numOfPackets++;
		return 0;
	}

	static int popPacketFromPacketQueue(PacketQueue* pPQ, AVPacket* pPkt) {
		AVPacketList* pPktList;
		pPktList = pPQ->pFirstPkt;
		if (pPktList) {
			pPQ->pFirstPkt = pPktList->next;
			if (!pPQ->pFirstPkt) {
				pPQ->pLastPkt = NULL;
			}
			pPQ->numOfPackets--;
			*pPkt = pPktList->pkt;
			av_free(pPktList);
			return 0;
		}
		return -1;
	}
};

#endif
