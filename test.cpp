#include "Player.h"
#include "AudioPlayer.h"
#include<string>
using namespace std;

char filepath[100] = "D:/TBBT.mp4";

int audioThread(void* opaque) {

	AudioPlayer audio;
	audio.openFile(filepath, AVMEDIA_TYPE_AUDIO);
	audio.audioSetting();
	audio.setAudioSDL();
	audio.play();
	audio.Player_Quit();
	return 0;
}

int main(int argc, char* argv[])
{
	/*avformat_network_init();
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
	}
	//SDL_Thread* video_tid = SDL_CreateThread(videoThread, NULL, NULL);
	SDL_Thread* audio_tid = SDL_CreateThread(audioThread, NULL, NULL);
	while (true)
	{
	}*/


	cout << "please input url:";
	//cin >> filepath;
	AudioPlayer audio;
	audio.openFile(filepath, AVMEDIA_TYPE_AUDIO);
	audio.audioSetting();
	//audio.setAudioSDL();
	audio.playByOpenAL();
	audio.~AudioPlayer();
	audio.Player_Quit();
	while (true)
	{
	}
	return 0;
}