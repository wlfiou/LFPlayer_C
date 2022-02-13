#include "lfmuxtask.h"


extern "C"{
    #include <libavformat/avformat.h>
}
using namespace std;
void LFMuxTask::Do(AVPacket *pkt){
    pkts_.Push(pkt);
    Next(pkt);
}
void LFMuxTask ::Main(){
    lmue_.WriteHead();
    //找到关键帧
       while (!is_exit_)
       {
           unique_lock<mutex> lock(mux_);
           auto pkt = pkts_.Pop();
           if (!pkt)
           {
               MSleep(1);
               continue;
           }
           if(pkt->stream_index == lmue_.video_index()
               && pkt->flags & AV_PKT_FLAG_KEY) //关键帧
           {
               lmue_.Write(pkt);
               av_packet_free(&pkt);
               break;
           }
           //丢掉非视频关键帧
           av_packet_free(&pkt);
       }


       while (!is_exit_)
       {
           unique_lock<mutex> lock(mux_);
           auto pkt = pkts_.Pop();
           if (!pkt)
           {
               MSleep(1);
               continue;
           }

           lmue_.Write(pkt);
           cout << "W"<< flush;

           av_packet_free(&pkt);
       }
       lmue_.WriteEnd();
       lmue_.set_c(nullptr);
}
bool LFMuxTask::Open(const char *url,
                     AVCodecParameters *video_para,
                     AVRational *video_time_base,
                     AVCodecParameters *audio_para,
                     AVRational *audio_time_base){

    auto c = lmue_.Open(url,video_para,audio_para);
    if(!c){
        return false;
    }
    lmue_.set_c(c);
    lmue_.set_src_video_time_base(video_time_base);
    lmue_.set_src_audio_time_base(audio_time_base);
    return true;


}
LFMuxTask::LFMuxTask()
{

}
