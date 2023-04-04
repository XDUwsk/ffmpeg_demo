#include <iostream>
 
extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/dict.h"
#include "libavutil/opt.h"
#include "libavutil/timestamp.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
};

int main()
{
	//av_register_all();
	//avformat_network_init();
 
    AVFormatContext* ifmt_ctx = NULL;
	const char* inputUrl = "/home/firefly/ffmpeg_workspace/media/4.mp4";
 
	///打开输入的流
	int ret = avformat_open_input(&ifmt_ctx, inputUrl, NULL, NULL);
	if (ret != 0)
	{
		printf("Couldn't open input stream.\n");
		return -1;
	}
 
	//查找流信息
	if (avformat_find_stream_info(ifmt_ctx, NULL) < 0)
	{
		printf("Couldn't find stream information.\n");
		return -1;
	}
 
    //输出的文件
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ofmt_ctx = NULL;
    const char* out_filename = "4_out.flv";
 
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx) 
    {
        return -1;
    }
 
    int stream_mapping_size = ifmt_ctx->nb_streams;
 
    //为数组分配内存
    int* stream_mapping = (int *)av_mallocz_array(stream_mapping_size, sizeof(*stream_mapping));
    if (!stream_mapping) 
    {
        return -1;
    }
 
    int stream_index = 0;
    ofmt = ofmt_ctx->oformat;
    for (int i = 0; i < ifmt_ctx->nb_streams; i++) 
    {
        //输出的流
        AVStream* out_stream = NULL;
 
        //输入的流 视频、音频、字幕等
        AVStream* in_stream = ifmt_ctx->streams[i];
        AVCodecParameters* in_codecpar = in_stream->codecpar;
        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO && in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO && in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) 
        {
            stream_mapping[i] = -1;
            continue;
        }
        stream_mapping[i] = stream_index++;
 
        //创建一个新的流
        out_stream = avformat_new_stream(ofmt_ctx, NULL); 
        if (!out_stream) 
        {
            return -1;
        }
 
        //复制输入的流信息到输出流中
        ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (ret < 0) 
        {
            return -1;
        }
        out_stream->codecpar->codec_tag = 0;
    }
 
    if (!(ofmt->flags & AVFMT_NOFILE)) 
    {
        //打开输出文件
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE); 
        if (ret < 0) 
        {
            return -1;
        }
    }
 
    //写入头
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) 
    {
        return -1;
    }
 
    AVPacket pkt;
    while (1) 
    {
        AVStream* in_stream = NULL;
        AVStream* out_stream = NULL;
 
        //从输入流中读取数据到pkt中
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;
 
        in_stream = ifmt_ctx->streams[pkt.stream_index];
        if (pkt.stream_index >= stream_mapping_size || stream_mapping[pkt.stream_index] < 0) 
        {
            av_packet_unref(&pkt);
            continue;
        }
        pkt.stream_index = stream_mapping[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
 
        /* copy packet */
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
 
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) 
        {
            fprintf(stderr, "Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
    }
 
    //写文件尾
    av_write_trailer(ofmt_ctx);
 
    //关闭
    avformat_close_input(&ifmt_ctx);
 
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
 
    avformat_free_context(ofmt_ctx);
    av_freep(&stream_mapping);
    if (ret < 0 && ret != AVERROR_EOF)
    {
        return -1;
    }
 
    return 0;
}
 
