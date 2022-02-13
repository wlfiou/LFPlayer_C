#include "lfdemux.h"
#include <iostream>
#include<thread>
using namespace std;

extern "C"{
    #include <libavformat/avformat.h>
}
void PrintErr(int err);
#define CERR(err) if(err != 0){PrintErr(err);return 0;}
LFDemux::LFDemux()
{

}

AVFormatContext *LFDemux::Open(const char *url){
    AVFormatContext *c = nullptr;

    AVDictionary *opts = nullptr;
    //
//    av_dict_set(&opts,"rtsp_transport","tcp",0);//传输媒体流协议为tcp，默认为udp
    av_dict_set(&opts,"stimeout","1000000",0);//链接超时1秒 -stimeout 设置在你要解析的url之后是没有作用的
    //打开封装上下文
    auto re = avformat_open_input(&c,url,
                                  nullptr,//封装器的格式 可以根据后缀或者文件头 自动探测
                                  &opts//参数设置 ，rtsp 需要设置
                                  );


    if(opts){
        av_dict_free(&opts);
    }
    CERR(re);
    //获取媒体信息
    re = avformat_find_stream_info(c,nullptr);
    //Could not find codec parameters for stream 1 (Video: h264, none): unspecified size
    CERR(re);
    av_dump_format(c,0,url,0);
return c;
}
bool LFDemux::Read(AVPacket *pkt){
    unique_lock<mutex>lock(mux_);
    if(!con_){
        return false;
    }
    //读出来完整一帧  一般这里在没连接成功时会有阻塞
    auto re = av_read_frame(con_,pkt);
    CERR(re);

    //计时 用于超时判断
    last_time_ = NowMs();

    return true;
}

bool LFDemux::Seek(long long pts, int stream_index){
    unique_lock<mutex>lock(mux_);
    if(!con_){
        return false;
    }
    //调到pts某一帧
    auto re= av_seek_frame(con_,stream_index,pts,
                           AVSEEK_FLAG_FRAME|AVSEEK_FLAG_BACKWARD
                           );
    CERR(re);
    return true;
}
















