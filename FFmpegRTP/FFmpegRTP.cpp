extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
}
#include <iostream>
#include <windows.h>
#include <stdio.h>

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")

// 代码来源于 https://stackoverflow.com/questions/46352604/writing-image-to-rtp-with-ffmpeg
//Mainly based on https://stackoverflow.com/questions/40825300/ffmpeg-create-rtp-stream
int main()
{
	//Init ffmpeg
	// avcodec_register_all();
	av_register_all();
	avformat_network_init();

	//Init the codec used to encode our given image
	AVCodecID codecID = AV_CODEC_ID_MPEG4;
	AVCodec* codec;
	AVCodecContext* codecCtx;

	codec = avcodec_find_encoder(codecID);
	codecCtx = avcodec_alloc_context3(codec);

	//codecCtx->bit_rate      = 400000;
	codecCtx->width = 352;
	codecCtx->height = 288;

	codecCtx->time_base.num = 1;
	codecCtx->time_base.den = 25;
	codecCtx->gop_size = 25;
	codecCtx->max_b_frames = 1;
	codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;

	if (codecID == AV_CODEC_ID_H264)
	{
		av_opt_set(codecCtx->priv_data, "preset", "ultrafast", 0);
		av_opt_set(codecCtx->priv_data, "tune", "zerolatency", 0);
	}

	avcodec_open2(codecCtx, codec, NULL);

	//Init the Frame containing our raw data
	AVFrame* frame;

	frame = av_frame_alloc();
	frame->format = codecCtx->pix_fmt;
	frame->width = codecCtx->width;
	frame->height = codecCtx->height;
	av_image_alloc(frame->data, frame->linesize, frame->width, frame->height, codecCtx->pix_fmt, 32);

	//Init the format context
	AVFormatContext* fmtCtx = avformat_alloc_context();
	AVOutputFormat*  format = av_guess_format("rtp", NULL, NULL);
	avformat_alloc_output_context2(&fmtCtx, format, format->name, "rtp://127.0.0.1:49990");

	avio_open(&fmtCtx->pb, fmtCtx->filename, AVIO_FLAG_WRITE);

	//Configure the AVStream for the output format context
	struct AVStream* stream = avformat_new_stream(fmtCtx, codec);

	avcodec_parameters_from_context(stream->codecpar, codecCtx);
	stream->time_base.num = 1;
	stream->time_base.den = 25;

	/* Rewrite the header */
	avformat_write_header(fmtCtx, NULL);

	/* Write a file for VLC */
	char buf[200000];
	AVFormatContext *ac[] = { fmtCtx };
	av_sdp_create(ac, 1, buf, 20000);
	printf("sdp:\n%s\n", buf);
	FILE* fsdp = fopen("test.sdp", "w");
	fprintf(fsdp, "%s", buf);
	fclose(fsdp);

	system("PAUSE");
	system("start "" \"C:\\Program Files\\VideoLAN\\VLC\\vlc.exe\" test.sdp");

	AVPacket pkt;
	int j = 0;
	for (int i = 0; i < 10000; i++)
	{
		Sleep(1e3 / 25);

		fflush(stdout);
		av_init_packet(&pkt);
		pkt.data = NULL;    // packet data will be allocated by the encoder
		pkt.size = 0;

		/* prepare a dummy image */
		/* Y */
		for (int y = 0; y < codecCtx->height; y++)
			for (int x = 0; x < codecCtx->width; x++)
				frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;

		for (int y = 0; y < codecCtx->height / 2; y++)
			for (int x = 0; x < codecCtx->width / 2; x++)
			{
				frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
				frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
			}

		/* Which frame is it ? */
		frame->pts = i;

		/* Send the frame to the codec */
		avcodec_send_frame(codecCtx, frame);

		/* Use the data in the codec to the AVPacket */
		switch (avcodec_receive_packet(codecCtx, &pkt))
		{
		case AVERROR_EOF:
			printf("Stream EOF\n");
			break;

		case AVERROR(EAGAIN):
			printf("Stream EAGAIN\n");
			continue;

		default:
			printf("Write frame %3d (size=%5d)\n", j++, pkt.size);
			av_packet_rescale_ts(&pkt, codecCtx->time_base, stream->time_base);

			/* Write the data on the packet to the output format  */
			av_interleaved_write_frame(fmtCtx, &pkt);

			/* Reset the packet */
			av_packet_unref(&pkt);
			continue;
		}


	}

	// end
	avcodec_send_frame(codecCtx, NULL);

	//Free everything
	av_free(codecCtx);
	av_free(fmtCtx);

	return 0;
}