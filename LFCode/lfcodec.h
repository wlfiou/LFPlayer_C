#ifndef LFCODEC_H
#define LFCODEC_H

#include <mutex>
#include<vector>

struct AVCodecContext ;
struct AVFrame;
struct AVPacket;

void PrintErr(int re);

class LFCodec
{
public:
    LFCodec();
    ///////////
    /// \brief Create
    /// \param codec_id   编码器id
    /// \param isencode
    /// \return
    ///创建编解码上下文  失败返回nullptr
    static AVCodecContext *Create(int codec_id,bool isencode);
    /////////
    /// \brief set_c
    /// \param c
    ///设置对象编码器上下文  如果c_不为nullptr，则先清理资源
    void set_c(AVCodecContext * c);
    ////////////
    /// \brief SetOpt
    /// \param key
    /// \param val
    /// \return
    ///设置编码参数 线程安全
    bool SetOpt(const char *key,const char * val);

    bool SetOpt(const char *key,const int val);
    ////////
    /// \brief Open
    /// \return
    ///打开编码器
    bool Open();
    /////////
    /// \brief CreateFrame
    /// \return
    ///根据AVCodecContext 创建一个AVFrame，需要调用者释放
   AVFrame*  CreateFrame();

protected:
   AVCodecContext *con_ = nullptr;
   std::mutex mux_;


};

#endif // LFCODEC_H
