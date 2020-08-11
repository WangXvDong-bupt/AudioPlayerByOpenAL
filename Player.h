#pragma once
#ifndef PLAYER_H
#define PLAYER_H

#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

extern "C"
{
	//封装格式
#include "libavformat/avformat.h"
	//解码
#include "libavcodec/avcodec.h"
	//缩放
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
	//播放
#include "SDL2/SDL.h"
};

class Player
{
public:
	Player();
	int Player_Quit();
	int openFile(char* filepath, int type);
	virtual int play() = 0;
	AVCodecContext* pCodeCtx = nullptr;
	AVFormatContext* pFormatCtx = nullptr;
	
	AVCodec* pCodec = nullptr;
	int index;
};

#endif
