#ifndef LFSDL_H
#define LFSDL_H

#include "lfvideoview.h"


struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class LFSDL : public LFVideoView
{
public:
    LFSDL();

    ///
    /// \brief Init
    /// \param w 窗口宽度
    /// \param h 窗口高度
    /// \param fmt 绘制像素的格式
    /// \param win_id 窗口句柄 如果为空 创建窗口
    /// \return  是否创建成功
    ///
    bool Init(int w,int h,Format fmt = RGBA,
                                  void * win_id = nullptr) override;

    ///
    /// \brief Draw
    /// \param data 需要渲染的二进制数据
    /// \param linsize 一行数据的字节数 YUV420P就是 Y一行的字节数据
    /// 如果linesize <= 0 就根据宽和高和像素格式自动计算出大小
    /// \return
    ///
    bool Draw(const unsigned  char *data,int linsize = 0) override;
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
    bool Draw(
               const unsigned char *y,int y_pitch,
               const unsigned char *u,int u_pitch,
               const unsigned char *v,int v_pitch
               ) override;



    void Close() override;
    bool IsExit() override;
private:
    SDL_Window *_win = nullptr;
    SDL_Renderer *_render = nullptr;
    SDL_Texture *_texture = nullptr;
};

#endif // LFSDL_H
