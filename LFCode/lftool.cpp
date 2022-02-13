#include "lftool.h"
#include <sstream>
using namespace std;
extern "C"{
 #include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/rational.h>
}

void LFreeFrame(AVFrame **frame){
    if(!frame || !(*frame)) return;
    av_frame_free(frame);
}

void MSleep(unsigned int ms){
    auto beg = clock();
    for(int i = 0;i<ms;i++){
        this_thread::sleep_for(1ms);
        if((clock() - beg)/(CLOCKS_PER_SEC/1000) >= ms){
            break;
        }
    }
}
///
/// \brief NowMs
/// \return
///  当前的毫秒数
long long NowMs(){
    return clock();
}

long long LFRescale(long long pts,
                    AVRational *src_time_base,
                    AVRational *des_time_base){
    cout << (src_time_base->den)<<"audio:"<<(src_time_base->num)<<endl;
    cout << (des_time_base->den)<<"video:"<<(des_time_base->num)<<endl;
    return av_rescale_q(pts,*src_time_base,*des_time_base);
}


void LFTread::start(){
    unique_lock<mutex>lock(mux_);
    static int i = 0;
    i++;
    index_ = i;
    is_exit_ = false;
    th_ = thread(&LFTread::Main,this);
    stringstream ss;
    ss <<"lstart()"<<index_ <<":classname:"<< typeid (*this).name();
    LOGINFO(ss.str());
}
//停止线程 （设置退出标志 等待线程退出）
void LFTread::stop(){
    stringstream ss;
    ss <<"LFTread::stop begin" << index_;
    LOGINFO(ss.str());
    is_exit_ = true;
    if(th_.joinable()){
        //判断子线程是否可以等待
        th_.join();//等待子线程退出
    }
    ss.str("");
    ss <<"LFTread::stop end" << index_;
    LOGINFO(ss.str());
}


LFPara *LFPara::Create(){
    return new LFPara;
}
LFPara::~LFPara(){
    if(para){
        avcodec_parameters_free(&para);
    }
    if(time_base){
        delete time_base;
        time_base = nullptr;
    }
}
// 私有是禁止创建栈中对象的
LFPara ::LFPara(){
    para = avcodec_parameters_alloc();
    time_base = new AVRational();
}


AVPacket *LFAVPacketList::Pop(){
    unique_lock<mutex>lock(mux_);

    if(pkts_.empty()){
        return nullptr;
    }
    auto pkt = pkts_.front();
    pkts_.pop_front();
    return pkt;

}


void LFAVPacketList::Push(AVPacket *pkt){
    unique_lock<mutex>lock(mux_);
    auto p = av_packet_alloc();
    av_packet_ref(p,pkt);//引用计数 减少数据复制 线程安全
    pkts_.push_back(p);
    //超出最大控件 清理数据到关键帧位置
    if(pkts_.size() > max_packets_){
           //处理第一针
        if(pkts_.front()->flags & AV_PKT_FLAG_KEY){//关键帧
            av_packet_free(&pkts_.front());//清理
            pkts_.pop_front();
            return;
        }

        while (!pkts_.empty()) {
            if(pkts_.front()->flags & AV_PKT_FLAG_KEY){
                return;
            }
            av_packet_free(&pkts_.front());
            pkts_.pop_front();
        }


    }

}
int LFAVPacketList::Size(){
    unique_lock<mutex>lock(mux_);
    return pkts_.size();
}
void LFAVPacketList::Clear(){
    unique_lock<mutex>lock(mux_);
    while (!pkts_.empty()) {
        av_packet_free(&pkts_.front());
        pkts_.pop_front();
    }
}

LFTool::LFTool()
{

}
