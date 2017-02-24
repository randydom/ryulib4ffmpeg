#pragma once


// Error Code
#define ERROR_GENERAL -1
#define ERROR_CAN_NOT_OPEN_FILE -2
#define ERROR_CAN_NOT_FIND_STREAMINFO -3
#define ERROR_CAN_NOT_FIND_VIDOESTREAM -4
#define ERROR_CAN_NOT_FIND_AUDIOSTREAM -5
#define ERROR_CAN_NOT_OPEN_VIDEOCODEC -6
#define ERROR_CAN_NOT_OPEN_AUDIOCODEC -7
#define ERROR_CAN_NOT_OPEN_AUDIO_RESAMPLER -8

// PacketType
#define UNKNOWN_PACKET 0
#define VIDEO_PACKET 1
#define AUDIO_PACKET 2


extern "C" __declspec(dllimport) void *open_stream(char *filename, int *error_code);
extern "C" __declspec(dllimport) void close_stream(void *handle);
extern "C" __declspec(dllimport) void *open_audio(void *stream, int *error_code);
extern "C" __declspec(dllimport) void close_audio(void *handle);
extern "C" __declspec(dllimport) void *open_video(void *stream, int *error_code);
extern "C" __declspec(dllimport) void close_video(void *handle);
extern "C" __declspec(dllimport) void *read_frame(void *stream);
extern "C" __declspec(dllimport) void release_frame(void *frame);
extern "C" __declspec(dllimport) void decode_audio(void *handle, void *frame, void *data_out, int *size_out);
extern "C" __declspec(dllimport) int decode_video(void *handle, void *frame, void *bitmap);
extern "C" __declspec(dllimport) void flush_video_buffer(void *handle);
extern "C" __declspec(dllimport) void set_position(void *stream, int position);
extern "C" __declspec(dllimport) int get_duration(void *stream);
extern "C" __declspec(dllimport) int get_video_width(void *handle);
extern "C" __declspec(dllimport) int get_video_height(void *handle);
extern "C" __declspec(dllimport) int get_sample_rate(void *handle);
extern "C" __declspec(dllimport) int get_channels(void *handle);
extern "C" __declspec(dllexport) int get_frame_type(void *frame);
extern "C" __declspec(dllimport) int get_frame_position(void *frame);
