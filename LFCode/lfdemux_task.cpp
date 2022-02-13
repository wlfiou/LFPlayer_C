#include "lfdemux_task.h"
#include <iostream>
using namespace std;

extern "C"{
#include <libavformat/avformat.h>
}

LFDemux_task::LFDemux_task()
{

}
bool LFDemux_task::Open(std::string url, int timeout_ms){
    LOGDEBUG("LFDemuxTast::open begin!");
    demux_.set_c(nullptr);//断开之前的链接
    this->url_ = url;
    this->timeout_ms = timeout_ms;

    auto c = demux_.Open(url.c_str());
    if(!c){
        return false;
    }


    demux_.set_c(c);
    demux_.set_time_out_ms(timeout_ms);
    LOGDEBUG("LFDemuxTask::open end");
    return true;

}

void LFDemux_task::Main(){
    AVPacket pkt;

    while (!is_exit_) {
        cout <<"is_exit_:"<< is_exit_<<endl;
          if(!demux_.Read(&pkt)){
              cout << "-"<< flush;
              if(!demux_.is_connect()){
                  //断线重连
                  Open(url_,timeout_ms);
              }
              this_thread::sleep_for(1ms);
              continue;

          }
          cout << "." << flush;

          if(syn_type_ == LFSYN_VIDEO && pkt.stream_index == demux_.video_index()){
              auto dur = demux_.RescaleToMs(pkt.duration,pkt.stream_index);
              if(dur <= 40){
                  dur = 40; //25帧
              }
              MSleep(dur);
          }

          Next(&pkt);
          av_packet_unref(&pkt);
          this_thread::sleep_for(1ms);
    }

}
