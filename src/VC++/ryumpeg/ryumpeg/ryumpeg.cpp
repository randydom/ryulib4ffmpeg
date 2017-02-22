// ryumpeg.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "yuvTools.h"


extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
}


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

typedef struct _Frame {
	char frame_type;
	int position;
	AVPacket *packet;
} Frame;

typedef struct _FFStreamHandle {
	AVFormatContext *ctx;

	int video_stream;
	int audio_stream;
} FFStreamHandle;

typedef struct _FFAudioHandle {
	AVCodecContext *ctx;
	AVCodec *codec;

	SwrContext *swr;
} FFAudioHandle;

typedef struct _FFVideoHandle {
	AVCodecContext *ctx;
	AVCodec *codec;

	int video_width;
	int video_height;

	AVFrame *frame;
} FFVideoHandle;


void trace(const char* format, ...)
{
	char buffer[4096];
	va_list vaList;
	va_start(vaList, format);
	_vsnprintf_s(buffer, 4096, format, vaList);
	va_end(vaList);
	OutputDebugStringA(buffer);
}

FFStreamHandle *create_stream_handle()
{
	FFStreamHandle *handle = (FFStreamHandle *) malloc(sizeof(FFStreamHandle));
	handle->ctx = NULL;
	handle->video_stream = -1;
	handle->audio_stream = -1;
	return handle;
}

void destroy_stream_handle(FFStreamHandle *handle)
{
	if (handle == NULL) return;
	if (handle->ctx != NULL) avformat_close_input(&handle->ctx);
	free(handle);
}

int find_stream_index(FFStreamHandle *handle)
{
	int i;

	for (i = 0; i < handle->ctx->nb_streams; i++) {
		if ((handle->ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) && (handle->video_stream < 0)) {
			handle->video_stream = i;
			break;
		}
	}

	if (-1 == handle->video_stream) return ERROR_CAN_NOT_FIND_VIDOESTREAM;

	for (i = 0; i < handle->ctx->nb_streams; i++) {
		if ((handle->ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) && (handle->audio_stream < 0)) {
			handle->audio_stream = i;
			break;
		}
	}

	if (-1 == handle->audio_stream) return ERROR_CAN_NOT_FIND_AUDIOSTREAM;

	printf("pHandle->videoStream: %d \n", handle->video_stream);
	printf("pHandle->audioStream: %d \n", handle->audio_stream);

	return 0;
}

extern "C" __declspec(dllexport) void *open_stream(char *filename, int *error_code) {
	*error_code = 0;

	FFStreamHandle *handle = create_stream_handle();

	av_register_all();
	avformat_network_init();

	if (avformat_open_input(&handle->ctx, filename, NULL, NULL) != 0) {
		*error_code = ERROR_CAN_NOT_OPEN_FILE;
		goto error;
	}

	if (avformat_find_stream_info(handle->ctx, NULL) < 0) {
		*error_code = ERROR_CAN_NOT_FIND_STREAMINFO;
		goto error;
	}

	*error_code = find_stream_index(handle);
	if (*error_code != 0) goto error;

	return handle;

error:
	destroy_stream_handle(handle);
	return NULL;
}

extern "C" __declspec(dllexport) void close_stream(FFStreamHandle *handle) {
	if (handle > 0) destroy_stream_handle(handle);
}

FFAudioHandle *create_audio_handle()
{
	FFAudioHandle *handle = (FFAudioHandle *) malloc(sizeof(FFAudioHandle));
	handle->ctx = NULL;
	return handle;
}

void destroy_audio_handle(FFAudioHandle *handle)
{
	if (handle == NULL) return;
	if (handle->ctx != NULL) avcodec_close(handle->ctx);
	free(handle);
}

int open_decoder(FFStreamHandle *stream, int index, AVCodecContext **ppCtx)
{
	AVCodecContext *pCodecCtx = stream->ctx->streams[index]->codec;

	AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	if (NULL == pCodec) return -1;

	*ppCtx = avcodec_alloc_context3(pCodec);
	if (avcodec_copy_context(*ppCtx, pCodecCtx) != 0) return -1;

	if (avcodec_open2(*ppCtx, pCodec, NULL) < 0) return -1;

	return 0;
}

int open_resampler(FFAudioHandle *handle)
{
	handle->swr = swr_alloc();

	uint64_t channel_layout = handle->ctx->channel_layout;
	if (channel_layout == 0) channel_layout = av_get_default_channel_layout(handle->ctx->channels);

	av_opt_set_int(handle->swr, "in_channel_layout", channel_layout, 0);
	av_opt_set_int(handle->swr, "in_sample_rate", handle->ctx->sample_rate, 0);
	av_opt_set_sample_fmt(handle->swr, "in_sample_fmt", handle->ctx->sample_fmt, 0);

	av_opt_set_int(handle->swr, "out_channel_layout", channel_layout, 0);
	av_opt_set_int(handle->swr, "out_sample_rate", handle->ctx->sample_rate, 0);
	av_opt_set_sample_fmt(handle->swr, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

	return swr_init(handle->swr);
}

extern "C" __declspec(dllexport) void *open_audio(FFStreamHandle *stream, int *error_code) {
	*error_code = 0;

	FFAudioHandle *handle = create_audio_handle();

	if (open_decoder(stream, stream->audio_stream, &handle->ctx) != 0) {
		*error_code = ERROR_CAN_NOT_OPEN_AUDIOCODEC;
		goto error;
	}

	if (open_resampler(handle) < 0) {
		*error_code = ERROR_CAN_NOT_OPEN_AUDIO_RESAMPLER;
		goto error;
	}

	return handle;

error:
	destroy_audio_handle(handle);
	return NULL;
}

extern "C" __declspec(dllexport) void close_audio(FFAudioHandle *handle) {
	if (handle > 0) destroy_audio_handle(handle);
}

FFVideoHandle *create_video_handle()
{
	FFVideoHandle *handle = (FFVideoHandle *) malloc(sizeof(FFVideoHandle));
	handle->ctx = NULL;
	handle->video_width = -1;
	handle->video_height = -1;
	handle->frame = av_frame_alloc();
	return handle;
}

void destroy_video_handle(FFVideoHandle *handle)
{
	if (handle == NULL) return;
	if (handle->ctx != NULL) avcodec_close(handle->ctx);
	if (handle->frame != NULL) av_frame_free(&handle->frame);
	free(handle);
}

extern "C" __declspec(dllexport) void *open_video(FFStreamHandle *stream, int *error_code) {
	*error_code = 0;

	FFVideoHandle *pHandle = create_video_handle();

	if (open_decoder(stream, stream->video_stream, &pHandle->ctx) != 0) {
		*error_code = ERROR_CAN_NOT_OPEN_VIDEOCODEC;
		goto error;
	}

	return pHandle;

error:
	destroy_video_handle(pHandle);
	return NULL;
}

extern "C" __declspec(dllexport) void close_video(FFVideoHandle *handle) {
	if (handle > 0) destroy_video_handle(handle);
}

extern "C" __declspec(dllexport) void *read_frame(FFStreamHandle *stream) {
	AVPacket *packet = (AVPacket *) malloc(sizeof(AVPacket));

	if (av_read_frame(stream->ctx, packet) >= 0) {
		Frame *frame = (Frame *) malloc(sizeof(Frame));
		frame->packet = packet;

		if (packet->stream_index == stream->audio_stream) {
			frame->frame_type = AUDIO_PACKET;
		} else if (packet->stream_index == stream->video_stream) {
			frame->frame_type = VIDEO_PACKET;
		} else {
			frame->frame_type = UNKNOWN_PACKET;
		}

		// frame->position = stream->ctx->streams[packet->stream_index]->cur_dts *
		// 	av_q2d(stream->ctx->streams[packet->stream_index]->time_base) * 1000;

		frame->position =
			packet->pts * av_q2d(stream->ctx->streams[packet->stream_index]->time_base) * 1000;

		return frame;
	} else {
		av_packet_unref(packet);
		free(packet);

		return NULL;
	}
}

extern "C" __declspec(dllexport) void release_frame(Frame *frame) {
	av_packet_unref(frame->packet);
	free(frame->packet);
	free(frame);
}

extern "C" __declspec(dllexport) void decode_audio(FFAudioHandle *handle, Frame *frame, void *data_out, int *size_out) {
	*size_out = 0;

	AVPacket packet;
	memset(&packet, 0, sizeof(packet));
	packet.size = frame->packet->size;
	packet.data = frame->packet->data;

	AVFrame *audio_frame = av_frame_alloc();

	int got_frame = 0;
	int converted_size = 0;

	while (packet.size > 0) {
		int len = avcodec_decode_audio4(handle->ctx, audio_frame, &got_frame, &packet);
		if (len < 0) return;

		packet.size -= len;
		packet.data += len;

		if (got_frame) {
			if (handle->ctx->sample_fmt != AV_SAMPLE_FMT_S16) {
				*size_out = audio_frame->nb_samples * handle->ctx->channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

				converted_size = swr_convert(
					handle->swr,
					(uint8_t **) &data_out, audio_frame->nb_samples,
					(const uint8_t **) audio_frame->extended_data, audio_frame->nb_samples
				);
			}
			else {
				*size_out = av_samples_get_buffer_size(
					NULL, handle->ctx->channels, audio_frame->nb_samples, AV_SAMPLE_FMT_S16, 1
				);
				memcpy(data_out, audio_frame->data[0], *size_out);
			}

			break;
		}
	}

	av_frame_free(&audio_frame);
}

extern "C" __declspec(dllexport) int decode_video(FFVideoHandle *handle, Frame *frame, void *bitmap) {
	AVPacket packet;
	memset(&packet, 0, sizeof(packet));
	packet.size = frame->packet->size;
	packet.data = frame->packet->data;

	int is_frame_finished = 0;
	int result = avcodec_decode_video2(handle->ctx, handle->frame, &is_frame_finished, &packet);
	if (!is_frame_finished) return FALSE;

	AVFrame *decoded_frame = av_frame_alloc();
	av_image_fill_linesizes(decoded_frame->linesize, handle->ctx->pix_fmt, handle->ctx->width);

	int ret = av_image_alloc(
		decoded_frame->data, decoded_frame->linesize, handle->ctx->width, handle->ctx->height,
		handle->ctx->pix_fmt, 1
	);
	if (ret < 0) {
		av_free(decoded_frame->data[0]);
		av_frame_free(&decoded_frame);
		return FALSE;
	}

	av_image_copy(
		decoded_frame->data, decoded_frame->linesize,
		(const uint8_t **) &handle->frame->data, handle->frame->linesize,
		handle->ctx->pix_fmt, handle->ctx->width, handle->ctx->height
	);

	decoded_frame->pict_type = handle->frame->pict_type;

	//yuv420_2_rgb565((uint8_t *) bitmap,
	//	decoded_frame->data[0],
	//	decoded_frame->data[1],
	//	decoded_frame->data[2],
	//	width,
	//	height,
	//	width,
	//	width >> 1,
	//	width << 1,
	//	yuv2rgb565_table,
	//	0
	//);
	I420ToARGB(
		(unsigned char *) decoded_frame->data[0], decoded_frame->linesize[0],
		(unsigned char *) decoded_frame->data[1], decoded_frame->linesize[1],
		(unsigned char *) decoded_frame->data[2], decoded_frame->linesize[2],
		(unsigned char *) bitmap,
		handle->ctx->width * 4,
		handle->ctx->width, handle->ctx->height
	);

	av_free(decoded_frame->data[0]);
	av_frame_free(&decoded_frame);

	return TRUE;
}

extern "C" __declspec(dllexport) void flush_video_buffer(FFVideoHandle *handle) {
	avcodec_flush_buffers(handle->ctx);
}

extern "C" __declspec(dllexport) void set_position(FFStreamHandle *stream, int position) {
	int64_t frameNumber = av_rescale(position,
		stream->ctx->streams[stream->video_stream]->time_base.den,
		stream->ctx->streams[stream->video_stream]->time_base.num);
	frameNumber /= 1000;
	int result = avformat_seek_file(stream->ctx, stream->video_stream, 0, frameNumber, frameNumber, AVSEEK_FLAG_FRAME);
}

extern "C" __declspec(dllexport) int get_duration(FFStreamHandle *stream) {
	return stream->ctx->duration * 1000 / AV_TIME_BASE;
}

extern "C" __declspec(dllexport) int get_video_width(FFVideoHandle *handle) {
	return handle->ctx->width;
}

extern "C" __declspec(dllexport) int get_video_height(FFVideoHandle *handle) {
	return handle->ctx->height;
}

extern "C" __declspec(dllexport) int get_sample_rate(FFAudioHandle *handle) {
	return handle->ctx->sample_rate;
}

extern "C" __declspec(dllexport) int get_channels(FFAudioHandle *handle) {
	return handle->ctx->channels;
}

extern "C" __declspec(dllexport) int get_frame_type(Frame *frame) {
	return frame->frame_type;
}

extern "C" __declspec(dllexport) int get_frame_position(Frame *frame) {
	return frame->position;
}