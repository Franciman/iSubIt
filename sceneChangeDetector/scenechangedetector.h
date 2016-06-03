#ifndef SCENECHANGEDETECTOR_H
#define SCENECHANGEDETECTOR_H

struct AVFormatContext;
struct AVCodecContext;
struct AVFilterContext;
struct AVFilterGraph;

class SceneChangeDetector
{
    AVFormatContext *FormatCtx;
    AVCodecContext *CodecCtx;
    AVFilterContext *BufferSinkCtx;
    AVFilterContext *BufferSrcCtx;
    AVFilterGraph *FilterGraph;
    int video_stream_idx;
public:
    SceneChangeDetector(int vsi, AVFormatContext *format_ctx, AVCodecContext *codec_ctx)
    {
        video_stream_idx = vsi;
        FormatCtx = format_ctx;
        CodecCtx = codec_ctx;
        init_filters();
    }
    ~SceneChangeDetector();

private:
    void init_filters();
};

#endif // SCENECHANGEDETECTOR_H
