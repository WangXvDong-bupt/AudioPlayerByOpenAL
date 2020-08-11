#include "Player.h"

Player::Player()
{
	//新版ffmpeg库不需集中初始化的组建
	//av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
	}
}


int Player::Player_Quit()
{
	SDL_Quit();
	avcodec_close(pCodeCtx);
	avformat_close_input(&pFormatCtx);
	return 0;
}



int Player::openFile(char* filepath, int type)
{
	int m = pFormatCtx->flags;
	//打开输入文件
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
		printf("Couldn't open input stream.\n");
		return -1;
	}

	//获取文件信息
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		printf("Couldn't find stream information.\n");
		return -1;
	}

	//找到对应的type所在的pFormatCtx->streams的索引位置
	index = -1;

	for (int i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codecpar->codec_type == type) {
			index = i;
			break;
		}
	if (index == -1) {
		printf("Didn't find a video stream.\n");
		return -1;
	}
	//获取解码器

	pCodeCtx = avcodec_alloc_context3(NULL);
	if (pCodeCtx == NULL)
	{
		printf("Could not allocate AVCodecContext\n");
		return -1;
	}
	avcodec_parameters_to_context(pCodeCtx, pFormatCtx->streams[index]->codecpar);


	//pCodeCtx = pFormatCtx->streams[index]->codecpar;
	pCodec = avcodec_find_decoder(pCodeCtx->codec_id);
	if (pCodec == NULL) {
		printf("Codec not found.\n");
		return -1;
	}

	//打开解码器
	if (avcodec_open2(pCodeCtx, pCodec, NULL) < 0) {
		printf("Could not open codec.\n");
		return -1;
	}
	return 0;
}