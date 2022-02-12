#include "RtspStreamHelper.h"
#include <direct.h>
#include <io.h>
#include <Basic/Guuid.h>


static std::string kVidoeType = ".mp4";
static std::string kVideoDir = "video";
static std::string kPictureDir = "picture";
RtspStreamHelper::RtspStreamHelper()
    : m_bExit(false)
    , m_pInputAVFormatCxt(nullptr)
    , m_pBitFilterAAC(nullptr)
    , m_pBitFilterH264(nullptr)
    , m_pOutputAVFormatCxt(nullptr)
    , m_bInputInited(false)
    , m_bOutputInited(false)
    , m_bFirstRun(true)
    , m_pCodecCtx(nullptr)
{
    // register all codecs
    avcodec_register_all();
    av_register_all();
    create_directory();
}

RtspStreamHelper::~RtspStreamHelper()
{
    StopRecvStream();
}

void RtspStreamHelper::StartRecvStream(const std::string& strUrlInput, const std::string& strOutputPath, CallbackFetchMat cbFetchMat)
{
    m_cbFetchMat = cbFetchMat;
    m_infoRtsp.strUrlInput = strUrlInput;
    if (strUrlInput.empty())
    {
        printf("Invalid stream input\n");
        return;
    }
    if (!open_input_stream(strUrlInput)) return;
    // save with picture and video
    open_output_stream(strOutputPath);
    m_thHandleFrame = std::thread(std::bind(&RtspStreamHelper::handle_packet, this));
    m_thDecode = std::thread(std::bind(&RtspStreamHelper::do_decode, this));
}

void RtspStreamHelper::StopRecvStream()
{
    m_bExit = true;
    m_cvPacket.notify_one();
    if (m_thDecode.joinable())
        m_thDecode.join();
    if (m_thHandleFrame.joinable())
        m_thHandleFrame.join();
    close_input_stream();
    close_output_stream();
}

void RtspStreamHelper::PushFrame(const cv::Mat& frame)
{
    if (m_cbFetchMat)
    {
        m_cbFetchMat(frame);
    }
    else
    {
        std::lock_guard<std::mutex> lock(m_mtFrame);
        m_listFrame.emplace_back(frame);
    }
}

bool RtspStreamHelper::PopFrame(cv::Mat& frame)
{
    std::lock_guard<std::mutex> lock(m_mtFrame);
    if(m_listFrame.empty()) return false;
    frame = m_listFrame.front();
    m_listFrame.pop_front();
	return true;
}

bool RtspStreamHelper::open_input_stream(const std::string& strUrlInput)
{
    if (m_pInputAVFormatCxt)
    {
        std::string strError = ("already has input avformat");
        return false;
    }

    m_pBitFilterAAC = av_bitstream_filter_init("aac_adtstoasc");
    if (!m_pBitFilterAAC)
    {
        std::string strError = ("cann't create aac_adtstoasc filter");
        return false;
    }

    m_pBitFilterH264 = av_bitstream_filter_init("h264_mp4toannexb");
    if (!m_pBitFilterH264)
    {
        std::string strError = ("cann't create h264_mp4toannexb filter");
        return false;
    }
    AVDictionary *pDict = NULL;
    m_pInputAVFormatCxt = avformat_alloc_context();
    av_dict_set(&pDict, "rtsp_transport", "tcp", 0);                //采用tcp传输
    av_dict_set(&pDict, "stimeout", "2000000", 0);
    m_pInputAVFormatCxt->flags |= AVFMT_FLAG_NONBLOCK;
    //打开多媒体数据并且获得信息
    int res = avformat_open_input(&m_pInputAVFormatCxt, m_infoRtsp.strUrlInput.c_str(), 0, &pDict);
    if (res < 0)
    {
        std::string strError = ("can not open file:" + m_infoRtsp.strUrlInput + ",errcode:" 
            + std::to_string(res) + ",err msg:" + av_make_error_string(m_tmpErrString, AV_ERROR_MAX_STRING_SIZE, res));
        return false;
    }
    //读取视音频数据并且获得信息
    if (avformat_find_stream_info(m_pInputAVFormatCxt, 0) < 0)
    {
        std::string strError = ("can not find stream info");
        return false;
    }
    //手工调试函数，看到pFormatCtx->streams的内容
    av_dump_format(m_pInputAVFormatCxt, 0, m_infoRtsp.strUrlInput.c_str(), 0);
    for (int i = 0; i < m_pInputAVFormatCxt->nb_streams; i++)
    {
        AVStream *in_stream = m_pInputAVFormatCxt->streams[i];
        printf("codec id: %d, URL: %s \n", in_stream->codec->codec_id, m_infoRtsp.strUrlInput.c_str());
        if (in_stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_infoRtsp.nVideoStreamIndex = i;

            m_infoRtsp.nCodedWidth = in_stream->codec->width;
            m_infoRtsp.nCodedHeight = in_stream->codec->height;
            m_pCodecCtx = m_pInputAVFormatCxt->streams[i]->codec;
            if (in_stream->avg_frame_rate.den != 0 && in_stream->avg_frame_rate.num != 0)
                m_infoRtsp.nFrameRate = in_stream->avg_frame_rate.num / in_stream->avg_frame_rate.den;//每秒多少帧 
            printf("video stream index: %d, width: %d, height: %d, FrameRate: %d\n", m_infoRtsp.nVideoStreamIndex, 
                in_stream->codec->width, in_stream->codec->height, m_infoRtsp.nFrameRate);
        }
        else if (in_stream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
            m_infoRtsp.nAudioStreamIndex = i;
    }
    //查找解码器
    AVCodec *pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
    if (nullptr == pCodec)
        return false;
    //初始化AVCodecContext
    if (avcodec_open2(m_pCodecCtx, pCodec, 0) < 0)
        return false;

    if (m_pCodecCtx->time_base.num > 1000 && m_pCodecCtx->time_base.den == 1)
        m_pCodecCtx->time_base.den = 1000;
    m_bInputInited = true;
    return true;
}

void RtspStreamHelper::close_input_stream()
{
    if (m_pInputAVFormatCxt)
        avformat_close_input(&m_pInputAVFormatCxt);
    if (m_pBitFilterAAC)
    {
        av_bitstream_filter_close(m_pBitFilterAAC);
        m_pBitFilterAAC = nullptr;
    }
    if (m_pBitFilterH264)
    {
        av_bitstream_filter_close(m_pBitFilterH264);
        m_pBitFilterH264 = nullptr;
    }
    m_bInputInited = false;
}

bool RtspStreamHelper::open_output_stream(const std::string& strOutputFile)
{
    if (m_pOutputAVFormatCxt)
    {
        printf("already has rtmp avformat \n");
        return false;
    }
    if (strOutputFile.empty())
    {
        printf("Invalid output file\n");
        return false;
    }
    int res = 0;
    if (!strOutputFile.empty())
    {
        std::string strFilename = generate_filename();
        res = avformat_alloc_output_context2(&m_pOutputAVFormatCxt, NULL, "mp4", strFilename.c_str());
        if (nullptr == m_pOutputAVFormatCxt)
        {
            printf("can not alloc output context \n");
            return false;
        }

        AVOutputFormat* fmt = m_pOutputAVFormatCxt->oformat;
        fmt->audio_codec = AV_CODEC_ID_AAC;     // video编码为AAC
        fmt->video_codec = AV_CODEC_ID_H264;
        for (int nIndex = 0; nIndex < m_pInputAVFormatCxt->nb_streams; ++nIndex)
        {
            AVStream *pInStream = m_pInputAVFormatCxt->streams[nIndex];
            AVStream *pOutStream = avformat_new_stream(m_pOutputAVFormatCxt, nullptr);
            //AVStream *pOutStream = avformat_new_stream(m_pOutputAVFormatCxt, pInStream->codec->codec);
            if (!pOutStream)
            {
                printf("can not new out stream");
                return false;
            }
            pOutStream->codecpar->codec_type = pInStream->codecpar->codec_type;
            //将输出流的编码信息复制到输入流
            //res = avcodec_copy_context(pOutStream->codec, pInStream->codec);
            res = avcodec_parameters_copy(pOutStream->codecpar, pInStream->codecpar);
            if (res < 0)
            {
                std::string strError = "can not copy context, url: " + m_infoRtsp.strUrlInput + ",errcode:" 
                    + std::to_string(res) + ",err msg:" + av_make_error_string(m_tmpErrString, AV_ERROR_MAX_STRING_SIZE, res);
                printf("%s \n", strError.c_str());
                return false;
            }
            if (m_pOutputAVFormatCxt->oformat->flags & AVFMT_GLOBALHEADER)
                pOutStream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            pOutStream->codecpar->codec_tag = 0;
        }
        av_dump_format(m_pOutputAVFormatCxt, 0, strFilename.c_str(), 1);
        if (!(fmt->flags & AVFMT_NOFILE))
        {
            res = avio_open(&m_pOutputAVFormatCxt->pb, strFilename.c_str(), AVIO_FLAG_WRITE);
            if (res < 0)
            {
                std::string strError = "can not open output io, file:" + strOutputFile + ",errcode:" + std::to_string(res) + ", err msg:" 
                    + av_make_error_string(m_tmpErrString, AV_ERROR_MAX_STRING_SIZE, res);
                printf("%s \n", strError.c_str());
                return false;
            }
        }

        res = avformat_write_header(m_pOutputAVFormatCxt, NULL);
        if (res < 0)
        {
            std::string strError = "can not write outputstream header, URL:" + strOutputFile + ",errcode:" + std::to_string(res) + ", err msg:" 
                + av_make_error_string(m_tmpErrString, AV_ERROR_MAX_STRING_SIZE, res);
            printf("%s \n", strError.c_str());
            m_bOutputInited = false;
            return false;
        }
        m_bOutputInited = true;
    }
    return true;
}

void RtspStreamHelper::close_output_stream()
{
    if (m_pOutputAVFormatCxt)
    {
        if (m_bOutputInited)
            av_write_trailer(m_pOutputAVFormatCxt);
        if (!(m_pOutputAVFormatCxt->oformat->flags & AVFMT_NOFILE))
        {
            if (m_pOutputAVFormatCxt->pb)
            {
                avio_close(m_pOutputAVFormatCxt->pb);
            }
        }
        avformat_free_context(m_pOutputAVFormatCxt);
        m_pOutputAVFormatCxt = nullptr;
    }
    m_bOutputInited = false;
}

void RtspStreamHelper::do_decode()
{
    // decode stream
    int nVideoFramesNum = 0;
    int64_t  timeFirstPts = 0;
    while (!m_bExit)
    {
        AVPacket pktFrame;
        av_init_packet(&pktFrame);
        int res = av_read_frame(m_pInputAVFormatCxt, &pktFrame);
        if (res < 0)  // error
        {
            if (AVERROR_EOF == res)
                printf("End of file \n");
            else
                printf("av_read_frame() got error: %d \n", res);
            break;
        }
        push_packet(pktFrame);
        //av_free_packet(&frame);
        ++nVideoFramesNum;
    }

    printf("Reading ended, read %d video frames \n", nVideoFramesNum);
}

void RtspStreamHelper::push_packet(const AVPacket& packet)
{
    {
        std::lock_guard<std::mutex>lock(m_mtLock);
        m_packetDecode.emplace_back(packet);
    }
    m_cvPacket.notify_one();
}

bool RtspStreamHelper::pop_packet(AVPacket& packet)
{
    std::lock_guard<std::mutex>lock(m_mtLock);
    if (m_packetDecode.empty())return false;
    packet = std::move(m_packetDecode.front());
    m_packetDecode.pop_front();
    return true;
}

void RtspStreamHelper::handle_packet()
{
    // handle frame of decode, save with video and picture
    int nFrame = 0;
    time_t tmStart = time(nullptr);
    while (!m_bExit)
    {
        {
            // block for packet
            std::mutex mtTmp;
            std::unique_lock<std::mutex> lock(mtTmp);
            m_cvPacket.wait(lock);
            if (m_bExit) break;
        }
        AVPacket frame;
        bool bSucc = pop_packet(frame);
        if (!bSucc)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        if (time(nullptr) - tmStart >= 1)   // check per second
            create_directory();
        // it must save picture first, or cause exception
        if (!save_picture(frame))continue;
        save_video(frame);
    }
    printf("Save finished\n");

}

void RtspStreamHelper::save_video(const AVPacket& packet)
{
    if (!m_bOutputInited || 0 == packet.buf->size)return;
    AVPacket pktFrame = packet;
    AVStream *pInStream = m_pInputAVFormatCxt->streams[pktFrame.stream_index];
    AVStream *pOutStream = m_pOutputAVFormatCxt->streams[pktFrame.stream_index];
    //转换PTS/DTS时序
    try {
        pktFrame.pts = av_rescale_q_rnd(pktFrame.pts, pInStream->time_base, pOutStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pktFrame.dts = av_rescale_q_rnd(pktFrame.dts, pInStream->time_base, pOutStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pktFrame.duration = av_rescale_q(pktFrame.duration, pInStream->time_base, pOutStream->time_base);
        pktFrame.pos = -1;
    }
    catch (const std::exception& e)
    {
        return;
    }
    catch (...)
    {
        printf("Unkonw error\n");
        return;
    }

    if (pInStream->codec->codec_type != AVMEDIA_TYPE_VIDEO 
        && pInStream->codec->codec_type != AVMEDIA_TYPE_AUDIO)
        return;

    if (pInStream->codec->codec_type == AVMEDIA_TYPE_VIDEO)  //视频
    {
        //printf("frame flag:%d\n", pktFrame.flags);
        //if (pktFrame.flags & AV_PKT_FLAG_KEY)return;
        // write the compressed frame to the output format
        // it'll cause pts mixed while the net not good
        int nError = av_interleaved_write_frame(m_pOutputAVFormatCxt, &pktFrame);
        if (nError != 0)
        {
            char tmpErrString[AV_ERROR_MAX_STRING_SIZE] = { 0 };
            av_make_error_string(tmpErrString, AV_ERROR_MAX_STRING_SIZE, nError);
            printf("Error: %d while writing video frame, %s\n", nError, tmpErrString);
        }
        int nSecs = pktFrame.pts * pInStream->time_base.num / pInStream->time_base.den;
        //printf("Frame time: %02d:%02d \n", nSecs / 60, nSecs % 60);
    }
    else if (pInStream->codec->codec_type == AVMEDIA_TYPE_AUDIO) //音频
    {
        // write the compressed frame to the output format
        int nError = av_interleaved_write_frame(m_pOutputAVFormatCxt, &pktFrame);
        if (nError != 0)
        {
            char tmpErrString[AV_ERROR_MAX_STRING_SIZE] = { 0 };
            av_make_error_string(tmpErrString, AV_ERROR_MAX_STRING_SIZE, nError);
            printf("Error: %d while writing audio frame, %s\n", nError, tmpErrString);
        }
    }
}

bool RtspStreamHelper::save_picture(const AVPacket& packet)
{
    if (0 == packet.buf->size)return false;
    static int nTask = 0;
    int bGotPicture;
    int ret = avcodec_decode_video2(m_pCodecCtx, m_infoPictureConvert.pFrame, &bGotPicture, &packet);//开始解码
    if (ret < 0)
    {
        printf("Decode Error.(解码错误),%d\n", ret);
        return false;
    }
    if (bGotPicture)//解码成功，got_picture返回任意非零值
    {
        if (!m_infoPictureConvert.m_bInited)
        {
            //初始化SwsContext
            m_infoPictureConvert.pImgConvertCtx = sws_getContext(m_pCodecCtx->width, m_pCodecCtx->height, 
                m_pCodecCtx->pix_fmt, m_pCodecCtx->width, m_pCodecCtx->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
            if (nullptr == m_infoPictureConvert.pImgConvertCtx)
            {
                fprintf(stderr, "Cannot initialize the conversion context!\n");
                return false;
            }

            m_infoPictureConvert.nByte = avpicture_get_size(AV_PIX_FMT_BGR24, m_pCodecCtx->width, m_pCodecCtx->height);
            m_infoPictureConvert.pBuffer = (uint8_t *)av_malloc(m_infoPictureConvert.nByte);
            avpicture_fill((AVPicture *)m_infoPictureConvert.pFrameRGB, m_infoPictureConvert.pBuffer, AV_PIX_FMT_BGR24, m_pCodecCtx->width, m_pCodecCtx->height); // allocator memory for BGR buffer  
            m_infoPictureConvert.pCvMat.create(cv::Size(m_pCodecCtx->width, m_pCodecCtx->height), CV_8UC3);
            m_infoPictureConvert.m_bInited = true;
        }
        if (nullptr == m_infoPictureConvert.pImgConvertCtx)return false;
        //处理图像数据
        int nRst = sws_scale(m_infoPictureConvert.pImgConvertCtx, m_infoPictureConvert.pFrame->data, m_infoPictureConvert.pFrame->linesize,
            0, m_pCodecCtx->height, m_infoPictureConvert.pFrameRGB->data, m_infoPictureConvert.pFrameRGB->linesize);
        memcpy(m_infoPictureConvert.pCvMat.data, m_infoPictureConvert.pBuffer, m_infoPictureConvert.nByte);
        PushFrame(m_infoPictureConvert.pCvMat);
    }
    else
    {
        printf("decode with picture failed\n");
    }
    return true;
}

std::string RtspStreamHelper::get_current_date()
{
    time_t ltime = time(NULL);
    std::tm ltm = { 0 };
    localtime_s(&ltm, &ltime);
    char buffer[128] = { 0 };
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &ltm);
    return buffer;
}

void RtspStreamHelper::create_directory()
{
    std::string strToday = get_current_date();
    if (strToday == m_strToday)return;
    if (0 == _mkdir(strToday.c_str())|| 0 == _access(strToday.c_str(), 0))
        m_strToday = strToday;
    std::string strVideoDir = m_strToday + "//" + kVideoDir;
    if(0 != _access(strVideoDir.c_str(), 0))
        _mkdir(strVideoDir.c_str());
    std::string strPictureDir = m_strToday + "//" + kPictureDir;
    if (0 != _access(strPictureDir.c_str(), 0))
        _mkdir(strPictureDir.c_str());
}

std::string RtspStreamHelper::generate_filename(int nType)
{
    std::string strFilename;
    switch (nType)
    {
    case 1: // 视频
        strFilename = m_strToday + "/" + kVideoDir + "/" + Basic::GenerateGuuid()  + ".mp4";
        break;
    default:    // 照片
        strFilename = m_strToday + "/" + kPictureDir + "/" + Basic::GenerateGuuid() + ".jpg";
        break;
    }

    return strFilename;
}

std::string RtspStreamHelper::get_current_path()
{
    char szPath[260] = { 0 };
    _getcwd(szPath, 260);

    return szPath;
}