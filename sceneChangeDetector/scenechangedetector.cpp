#include "scenechangedetector.h"

#include <stdint.h>

extern "C"
{
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>

#include <libavformat/avformat.h>
}

void SceneChangeDetector::init_filters()
{
    AVFilter *buffersrc = avfilter_get_by_name("buffer");
    AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    AVRational time_base = FormatCtx->streams[video_stream_idx]->time_base;

    FilterGraph = avfilter_graph_alloc();
    char args[512];
    snprintf(args, sizeof(args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d", CodecCtx->width, CodecCtx->height, CodecCtx->pix_fmt, time_base.num, time_base.den, CodecCtx->sample_aspect_ratio.num, CodecCtx->sample_aspect_ratio.den);

    int ret = avfilter_graph_create_filter(&BufferSrcCtx, buffersrc, "in", args, nullptr, FilterGraph);

    ret = avfilter_graph_create_filter(&BufferSinkCtx, buffersink, "out", NULL, NULL, FilterGraph);

    outputs->name = av_strdup("in");
    outputs->filter_ctx = BufferSrcCtx;
    outputs->pad_idx = 0;
    outputs->next = nullptr;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = BufferSinkCtx;
    inputs->pad_idx = 0;
    inputs->next = nullptr;

    const char *filters_descr="select=gt(scene\\,0.4)";

    avfilter_graph_parse_ptr(FilterGraph, filters_descr, &inputs, &outputs, nullptr);
    avfilter_graph_config(FilterGraph, nullptr);

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
}

SceneChangeDetector::~SceneChangeDetector()
{
    avfilter_graph_free(&FilterGraph);
}
