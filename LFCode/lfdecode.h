#ifndef LFDECODE_H
#define LFDECODE_H


struct AVBufferRef;

#include "lfcodec.h"
class LFDecode : public LFCodec
{
public:
    LFDecode();
    bool Send(const AVPacket *pkt);
    bool Recv(AVFrame *frame);
    std::vector<AVFrame *>End();

    ///////
    /// \brief InitHW
    /// \param type
    /// \return
    /// 初始化硬件加速 4 AV_HWDEVICE_TYPE_DXVA2
    bool InitHW(int type = 4);
};

#endif // LFDECODE_H
