# CMakeList.txt: PlayerByOpenAL 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)


project ("PlayerByOpenAL")
# --------------------------------------------------------------------------
# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-D__STDC_CONSTANT_MACROS")

include_directories("include")
link_directories("lib")


######################################
#  FFmpeg lib setting
######################################
#make sure ffmpeg/bin is in your PATH to RUN.

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
find_library(AVDEVICE_LIBRARY avdevice)

find_path(AVFILTER_INCLUDE_DIR libavfilter/avfilter.h)
find_library(AVFILTER_LIBRARY avfilter)

find_path(POSTPROC_INCLUDE_DIR libpostproc/postprocess.h)
find_library(POSTPROC_LIBRARY postproc)

find_path(SWRESAMPLE_INCLUDE_DIR libswresample/swresample.h)
find_library(SWRESAMPLE_LIBRARY swresample)

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_library(SWSCALE_LIBRARY swscale)


######################################
#  SDL2 lib setting
######################################
#make sure SDL2.dll is in your PATH.

set(SDL_INCLUDE_DIR "./include./SDL2")
set(SDL_LIBRARY ".lib/SDL2.lib")

find_path(SDL_INCLUDE_DIR "SDL/SDL.h")
find_library(SDL_LIBRARY SDL2.lib)


######################################
#  OpenAL lib setting
######################################
#make sure openAL32.dll is in your PATH.

#set(OpenAL_INCLUDE_DIR "C:/Program Files (x86)/OpenAL 1.1 SDK/include")
#set(OpenAL_LIBRARY "C:/Program Files (x86)/OpenAL 1.1 SDK/libs/Win64/OpenAL32.lib")

find_path(OpenAL_INCLUDE_DIR "OpenAL/al.h")
find_library(OpenAL_LIBRARY "OpenAL32.lib")


######################################
#  information
######################################





# 将源代码添加到此项目的可执行文件。
add_executable (PlayerByOpenAL   "Player.cpp" "AudioPlayer.cpp" "test.cpp")
set_property(TARGET PlayerByOpenAL PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "")



target_link_libraries(
	PlayerByOpenAL	#本项目的解决方案名称
	avcodec		#音视频编码核心库 , 用于音视频或图片编解码
	avdevice	#硬件采集 , 加速 , 显示 库
	avfilter	#音视频滤波器
	avformat	#音视频封装格式生成或解析 
	avutil		#该库中封装了一些公共的工具函数
	postproc	#封装了同步, 时间计算的简单算法
	swresample	#音视频编解码格式预设
	swscale		#原始视频格式转换库
	SDL2
	SDL2main
	SDL2test
	OpenAL32
)

# TODO: 如有需要，请添加测试并安装目标。
