#ifndef LFDEMUX_TASK_H
#define LFDEMUX_TASK_H

#include "lftool.h"
#include "lfdemux.h"

enum LFSYN_TYPE{
    LFSYN_NONE = 0,//不做同步
    LFSYN_VIDEO= 1,//根据视频同步
};

class LFDemux_task : public LFTread
{
public:
    LFDemux_task();

    int audio_index() { return demux_.audio_index(); }
    int video_index() { return demux_.video_index(); }

    void Main();

    ///
    /// \brief Open
    /// \param url
    /// \param timeout_ms  默认1s
    /// \return
    ///
    bool Open(std::string url,int timeout_ms = 1000);
//智能指针
    std::shared_ptr<LFPara>CopyVideoPara(){
        return demux_.CopyVideoPara();
    }
    std::shared_ptr<LFPara>CopyAudioPara(){
        return demux_.CopyAudioPara();
    }

    void set_syn_type(LFSYN_TYPE t){syn_type_ = t;}

private:
    LFDemux demux_;
    std::string url_;
    int timeout_ms = 0;//超时时间
    LFSYN_TYPE syn_type_ = LFSYN_NONE;//
};

#endif // LFDEMUX_TASK_H
