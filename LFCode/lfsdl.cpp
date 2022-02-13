#include "lfsdl.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <QDebug>
using namespace std;
LFSDL::LFSDL()
{

}

static bool InitVideo(){

    static bool isFirst = true;
    static mutex mux;
    unique_lock<mutex> sdl_lock(mux);

    if(!isFirst) return  true;
    isFirst = false;
    if(SDL_Init(SDL_INIT_VIDEO)){
        cout << SDL_GetError() << endl;
        return false;
    }
    //设定缩放算法，解决锯齿问题,线性插值算法
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    return true;
}

bool LFSDL::IsExit(){
    SDL_Event ev;
    SDL_WaitEventTimeout(&ev,1);
    if(ev.type == SDL_QUIT){
        return true;
    }
    return false;
}
void LFSDL::Close(){
    //确保线程安全
       unique_lock<mutex> sdl_lock(_mtx);
       if (_texture)
       {
           SDL_DestroyTexture(_texture);
           _texture = nullptr;
       }
       if (_render)
       {
           SDL_DestroyRenderer(_render);
           _render = nullptr;
       }
       if (_win)
       {
           SDL_DestroyWindow(_win);
           _win = nullptr;
       }
}

bool LFSDL::Init(int w, int h, Format fmt, void *win_id){
    if(w<=0 || h<=0) return false;

    InitVideo();

    //确保线程安全
    unique_lock<mutex> sdl_lock(_mtx);

    _width = w;
    _height = h;
    _fmt = fmt;

    if(_texture){
        SDL_DestroyTexture(_texture);
    }
    if(_render){
        SDL_DestroyRenderer(_render);
    }

    if(!_win){
        if(!win_id_){
            _win = SDL_CreateWindow("LFVideoView",
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    w,h,SDL_WINDOW_OPENGL
                                    );
        }else{
            _win = SDL_CreateWindowFrom(win_id_);
        }
    }

    if(!_win){
        cerr << SDL_GetError()<< endl;
        return false;
    }

    _render = SDL_CreateRenderer(_win,-1,SDL_RENDERER_ACCELERATED);
    if(!_render){
        cerr << SDL_GetError() << endl;
        return false;
    }

    //创建纹理
    unsigned int sdl_fmt = SDL_PIXELFORMAT_RGBA8888;

    switch (fmt) {
    case LFVideoView::RGBA:
        sdl_fmt = SDL_PIXELFORMAT_RGBA8888;
        break;
    case LFVideoView::BGRA:
        sdl_fmt = SDL_PIXELFORMAT_BGRA8888;
        break;
    case LFVideoView::ARGB:
        sdl_fmt = SDL_PIXELFORMAT_ARGB8888;
        break;
    case LFVideoView::YUV420P:
        sdl_fmt = SDL_PIXELFORMAT_IYUV;
        break;
    default:
        break;
    }

    _texture = SDL_CreateTexture(_render,sdl_fmt,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 w,h);
    if(!_texture){
        cerr << SDL_GetError()<< endl;
        return false;
    }

    return true;
}

bool LFSDL::Draw(const unsigned  char *data, int linsize){
    if(!data){
        return false;
    }

    unique_lock<mutex> sdl_lock(_mtx);
    if(!_texture || !_render || !_win||_width <=0 || _height <=0){
        return false;
    }
    if(linsize <= 0){
        switch (_fmt) {
        case LFVideoView::RGBA:
        case LFVideoView::ARGB:
        case LFVideoView::BGRA:
            linsize = _width*4;
            break;
        case LFVideoView::YUV420P:
            linsize = _width;
            break;
        default:
            break;
        }
    }

    if(linsize <= 0){
        return false;
    }
    //复制内存到显存

    auto re = SDL_UpdateTexture(_texture,NULL,data,linsize);

    if(re != 0){
        cout << SDL_GetError()<< endl;
        return false;
    }

    //清空屏幕
    SDL_RenderClear(_render);
    //纹理渲染到渲染器

//    if(_scale_w <=0)_scale_w = _width;
//    if(_scale_h <= 0)_scale_h = _height;

    SDL_Rect rect= {0,0,_scale_w,_scale_h};

    SDL_Rect* prect = nullptr;
        if (_scale_w > 0 )  //用户手动设置缩放
        {
            rect.x = 0; rect.y = 0;
            rect.w = _scale_w;//渲染的宽高，可缩放
            rect.h = _scale_h;
            prect = &rect;
        }

    re = SDL_RenderCopy(_render,_texture,NULL,prect);

    if(re != 0){
        cout << SDL_GetError()<< endl;
        return false;
    }
    SDL_RenderPresent(_render);
    return true;
}
bool LFSDL::Draw(
                 const unsigned char *y, int y_pitch,
                 const unsigned char *u, int u_pitch,
                 const unsigned char *v, int v_pitch){

    if(!y || !u ||!v){
        return false;
    }

    unique_lock<mutex> sdl_lock(_mtx);
    if(!_texture || !_render || !_win||_width <=0 || _height <=0){
        return false;
    }

    auto re = SDL_UpdateYUVTexture(_texture,NULL,
                                                           y,y_pitch,
                                                           u,u_pitch,
                                                           v,v_pitch);
    if(re != 0){
        cout << SDL_GetError() << endl;
        return  false;
    }

    //清空屏幕
    SDL_RenderClear(_render);
    //纹理渲染到渲染器

//    if(_scale_w <=0)_scale_w = _width;
//    if(_scale_h <= 0)_scale_h = _height;

    SDL_Rect rect= {0,0,_scale_w,_scale_h};

    SDL_Rect* prect = nullptr;
        if (_scale_w > 0 )  //用户手动设置缩放
        {
            rect.x = 0; rect.y = 0;
            rect.w = _scale_w;//渲染的宽高，可缩放
            rect.h = _scale_h;
            prect = &rect;
        }

    re = SDL_RenderCopy(_render,_texture,NULL,prect);

    if(re != 0){
        cout << SDL_GetError()<< endl;
        return false;
    }
    cout << "SDL_RenderPresent" << endl;
    SDL_RenderPresent(_render);
    return true;
}











