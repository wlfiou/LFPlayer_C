#ifndef LFTOOL_H
#define LFTOOL_H

#include <thread>
#include <iostream>
#include<mutex>
#include<list>
struct AVPacket;
struct AVCodecParameters;
struct AVRational;
struct AVFrame;

void LFreeFrame(AVFrame **frame);

enum LFLogLevel{
    LLOG_TYPE_DEBUG,
    LLOG_TYPE_INFO,
    LLOG_TYPE_ERROR,
    LLOG_TYPE_FATAL
};

#define LOG_MIN_LEVEL LLOG_TYPE_DEBUG
#define LLOG(s,level) if(level>=LOG_MIN_LEVEL) \
    std::cout<<level<<":"<<__FILE__<<":"<<__LINE__<<":\n"\
    <<s<<std::endl;
#define LOGDEBUG(s) LLOG(s,LLOG_TYPE_DEBUG)
#define LOGINFO(s) LLOG(s,LLOG_TYPE_INFO)
#define LOGERROR(s) LLOG(s,LLOG_TYPE_ERROR)



void MSleep(unsigned int ms);
//获取当前的时间戳 毫秒
long long NowMs();

//根据时间基数计算
long long LFRescale(long long pts,
                    AVRational *src_time_base,
                    AVRational *des_time_base);


class LFTread{
public:
    //////
    /// \brief start
    ///启动线程
    virtual void start();
    ///////
    /// \brief stop
    ///停止线程 （设置退出标志，等待线程退出）
    virtual void stop();
    //////
    /// \brief Do
    /// \param pkt
    ///启动任务  需要重载
    virtual void Do(AVPacket * pkt){};
    ///
    /// \brief Next
    /// \param pkt
    ///传递到下一个责任链
    virtual void Next(AVPacket *pkt){
        std::unique_lock<std::mutex>lock(mux_);
        if(nexr_){
            nexr_->Do(pkt);
        }
    }

    void set_next(LFTread *lft)
    {
        std::unique_lock<std::mutex>lock(mux_);
        nexr_ = lft;
    }


protected:
    //////
    /// \brief Main
    ///线程接入函数
    virtual void Main() = 0;
    //线程是否退出
    bool is_exit_ = false;

    //线程索引号
    int index_ = 0;

private:
    std::thread th_;
    std::mutex mux_;
    LFTread *nexr_ = nullptr;//责任链下一个节点
};

class LFPara{
public:
    AVCodecParameters *para = nullptr;
    AVRational *time_base = nullptr;

    //创建对象
    static LFPara *Create();
    ~LFPara();
    public:
    //私有是禁止创建栈中对象
    LFPara();

};



class LFAVPacketList{
public:
    AVPacket *Pop();
    void Push(AVPacket *pkt);
   int Size();
   void Clear();
private:
    std::list<AVPacket *>pkts_;
    int max_packets_= 100;//最大列表数量 超出的话清理

    std::mutex mux_;

};



























class LFTool
{
public:
    LFTool();
};

#endif // LFTOOL_H
