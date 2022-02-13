#ifndef LFVIDEOVIEW_H
#define LFVIDEOVIEW_H
///////////////////////////////////////
/// \brief The LFVideoView class
///视频渲染接口类
/// 隐藏方案可替代
/// 线程安全

#include <mutex>
#include <fstream>
#include "lftool.h"
struct AVFrame;
struct AVCodecParameters;
//void MSleep(unsigned int ms);
//long long NowMs();//返回现在的时间
class LFVideoView
{
public:

    enum Format{
        YUV420P = 0,
        ARGB = 25,
        RGBA = 26,
        BGRA = 28,

    };
    enum RenderType{
        SDL = 0
    };
    LFVideoView();

    static LFVideoView *create(RenderType type = SDL);


    ///
    /// \brief Init
    /// \param w 窗口宽度
    /// \param h 窗口高度
    /// \param fmt 绘制像素的格式
    /// \param win_id 窗口句柄 如果为空 创建窗口
    /// \return  是否创建成功
    ///
    virtual bool Init(int w,int h,Format fmt = RGBA,
                                  void * win_id = nullptr) = 0;

    bool Init(AVCodecParameters *para);

    ///
    /// \brief Draw
    /// \param data 需要渲染的二进制数据
    /// \param linsize 一行数据的字节数 YUV420P就是 Y一行的字节数据
    /// 如果linesize <= 0 就根据宽和高和像素格式自动计算出大小
    /// \return
    ///
    virtual bool Draw(const unsigned  char *data,int linsize = 0) = 0;
    ///
    /// \brief Draw 渲染YUV420P
    /// \param y Y分量的数据
    /// \param y_pitch  Y分量的数据大小
    /// \param u
    /// \param u_pitch
    /// \param v
    /// \param v_pitch
    /// \return
    ///
    virtual bool Draw(
            const unsigned char *y,int y_pitch,
            const unsigned char *u,int u_pitch,
            const unsigned char *v,int v_pitch
            ) = 0 ;


    virtual void Close() = 0;

    virtual bool IsExit() = 0;

    void Scale(int w, int h)
        {
            _scale_w = w;
            _scale_h = h;
        }

    bool DrawFrame(AVFrame *frame);

    int render_fps(){return render_fps_;}

    //打开文件
    bool Open(std::string filepath);
    //////////////////////////////////////
        /// 读取一帧数据，并维护AVFrame空间
        /// 每次调用会覆盖上一次数据
    AVFrame *Read();
    void set_win_id(void * win){ win_id_ = win ;}

protected:
    void* win_id_ = nullptr; //窗口句柄
    int render_fps_ = 0;
    int _width = 0; //纹理的宽高
    int _height = 0;
    Format _fmt = RGBA;
    std::mutex _mtx;//确保线程安全
    int _scale_w  = 0;//显示的大小
    int _scale_h = 0;

    long long beg_ms_ = 0;
    int count_ = 0;

private:
    std::ifstream ifs_;
    AVFrame *frame_;
};

#endif // LFVIDEOVIEW_H
