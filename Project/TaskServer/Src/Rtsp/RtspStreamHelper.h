#pragma once
#include <string>
#include <thread>
#include <list>
#include <mutex>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavdevice/avdevice.h>
    #include <libavformat/avio.h>
    #include <libavfilter/avfilter.h>
    #include <libavutil/imgutils.h>
}


class RtspStreamHelper
{
    using CallbackFetchMat = std::function<void(const cv::Mat& cvMat)>;
    // rtsp信息
    struct RtspStreamInfo
    {
        std::string strUrlInput;
        int nCodedWidth = 0;
        int nCodedHeight = 0;
        int nFrameRate = 25;
        int nVideoStreamIndex;
        int nAudioStreamIndex;
    };
    // 图片编码转换
    struct PictureConvertInfo
    {
        bool m_bInited = false;
        struct SwsContext *pImgConvertCtx = nullptr;
        uint8_t *pBuffer = nullptr;
        int nByte = 0;
        AVFrame *pFrame = nullptr;
        AVFrame *pFrameRGB = nullptr;
        cv::Mat pCvMat;
        PictureConvertInfo()
        {
            pFrame = av_frame_alloc();//分配内存
            pFrameRGB = av_frame_alloc();
        }
    };
public:
    RtspStreamHelper();
    virtual ~RtspStreamHelper();

    void StartRecvStream(const std::string& strUrlInput, const std::string& strOutputPath, CallbackFetchMat cbFetchMat);
    void StopRecvStream();

    void GetVideoSize(long & width, long & height)  //获取视频分辨率
    {
        width = m_infoRtsp.nCodedWidth;
        height = m_infoRtsp.nCodedHeight;
    }

    void PushFrame(const cv::Mat& frame);
    bool PopFrame(cv::Mat& frame);

private:
    // input
    bool open_input_stream(const std::string& strUrlInput);
    void close_input_stream();
    // output
    bool open_output_stream(const std::string& strOutputFile);
    void close_output_stream();
    void do_decode();
    void push_packet(const AVPacket& packet);
    bool pop_packet(AVPacket& packet);
    void handle_packet();
    void save_video(const AVPacket& packet);
    bool save_picture(const AVPacket& packet);
    std::string get_current_date();
    void create_directory();
    std::string generate_filename(int nType = 1);
    std::string get_current_path();

private:
    bool m_bExit;
    bool m_bFirstRun;
    CallbackFetchMat m_cbFetchMat;
    std::string m_strToday;
    RtspStreamInfo m_infoRtsp;
    AVFormatContext* m_pInputAVFormatCxt;
    AVCodecContext *m_pCodecCtx;
    AVBitStreamFilterContext* m_pBitFilterAAC;
    AVBitStreamFilterContext* m_pBitFilterH264;
    AVFormatContext* m_pOutputAVFormatCxt;

    char m_tmpErrString[64];
    bool m_bInputInited;
    bool m_bOutputInited;

    std::thread m_thDecode;
    std::thread m_thHandleFrame;
    std::mutex m_mtLock;
    std::condition_variable m_cvPacket;
    std::list<AVPacket> m_packetDecode;
    PictureConvertInfo m_infoPictureConvert;

    // cache the frame
    std::mutex m_mtFrame;
    std::list<cv::Mat> m_listFrame;

};