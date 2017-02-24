// vc+test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "RyuMPEG.h"


int main()
{
	int error_code;

	void *stream = open_stream("rtsp://184.72.239.149/vod/mp4:BigBuckBunny_115k.mov", &error_code);
	if (error_code != 0) {
		printf("open_stream error - %d \n", error_code);
		return 0;
	}

	printf("get_duration: %d \n", get_duration(stream));

	void *video = open_video(stream, &error_code);
	if (error_code != 0) {
		printf("open_video error - %d \n", error_code);
		return 0;
	}
	printf("get_video_width:  %d \n", get_video_width (video));
	printf("get_video_height: %d \n", get_video_height(video));

	void *bitmap = malloc(get_video_width(video) * get_video_height(video) * 4);

	void *audio = open_audio(stream, &error_code);
	if (error_code != 0) {
		printf("open_audio error - %d \n", error_code);
		return 0;
	}
	printf("get_channels:    %d \n", get_channels   (audio));
	printf("get_sample_rate: %d \n", get_sample_rate(audio));

	void *audio_buffer = malloc(16 * 1024);

	void *frame = read_frame(stream);
	while (frame != NULL) {
		printf("  - get_frame_position: %d \n", get_frame_position(frame));

		switch (get_frame_type(frame)) {
			case VIDEO_PACKET: {
				decode_video(video, frame, bitmap);
			} break;

			case AUDIO_PACKET: {
				int audio_size;
				decode_audio(audio, frame, audio_buffer, &audio_size);
				// TODO:
				if (audio_size > 0);
			} break;
		}

		release_frame(frame);
		frame = read_frame(stream);
	}

	close_video(video);
	close_audio(audio);
	close_stream(stream);

	return 0;
}

