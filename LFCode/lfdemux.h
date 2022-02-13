#ifndef LFDEMUX_H
#define LFDEMUX_H

#include "lfformat.h"

class LFDemux : public LFFormat
{
public:
    LFDemux();
    ////////
    /// \brief Open
    /// \param url
    /// \return
    ///打开解封装，解封装地址 支持rtsp
    static AVFormatContext *Open(const char* url);
    //////////
    /// \brief Read
    /// \param pkt
    /// \return
    ///读取一帧数据 是否成功
    bool Read(AVPacket *pkt);

    bool Seek(long long pts,int stream_index);
};

#endif // LFDEMUX_H
