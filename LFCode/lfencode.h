#ifndef LFENCODE_H
#define LFENCODE_H

#include "lfcodec.h"

class LFEncode : public LFCodec
{
public:
    LFEncode();
    //////////
    /// \brief Encodec
    /// \param frame
    /// \return
    ///编码数据 线程安全 每次创建新的AVpacket 需要用户手动清理AVpacket
    AVPacket *Encode(const AVFrame *frame);
    ///////////////
    /// \brief End
    /// \return
    ///返回所有编码缓存中的AVpacket
    std::vector<AVPacket *>End();
};

#endif // LFENCODE_H
