#ifndef VIDEO_H
#define VIDEO_H

#include "sdl2vlc_global.h"

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string>

#include "context.h"

extern "C"
{
    #include "SDL.h"
    #include "SDL_mutex.h"

    #include "vlc/vlc.h"
}

#define VIDEOWIDTH 320
#define VIDEOHEIGHT 240

// VLC prepares to render a video frame.
static void *lock(void *data, void **p_pixels)
{
    Context *c = (Context *)data;

    int pitch;
    SDL_LockMutex(c->mutex);
    SDL_LockTexture(c->texture, nullptr, p_pixels, &pitch);

    return nullptr; // Picture identifier, not needed here.
}

// VLC just rendered a video frame.
static void unlock(void *data, void *id, void *const *p_pixels)
{
    Context *c = (Context *)data;
    // We can also render stuff.
    /*
     * Things to render over video goes here
     *
    uint16_t *pixels = (uint16_t *)*p_pixels;

    int x, y;
    for(y = 10; y < 40; y++)
    {
        for(x = 10; x < 40; x++)
        {
            if(x < 13 || y < 13 || x > 36 || y > 36)
            {
                pixels[y * VIDEOWIDTH + x] = 0xffff;
            }
            else
            {
                // RV16 = 5+6+5 pixels per color, BGR.
                pixels[y * VIDEOWIDTH + x] = 0x02ff;
            }
        }
    }
    */

    SDL_UnlockTexture(c->texture);
    SDL_UnlockMutex(c->mutex);
}

// VLC wants to display a video frame.
static void display(void *data, void *id)
{
    Context *c = (Context *)data;

    SDL_Rect rect;
    rect.w = VIDEOWIDTH;
    rect.h = VIDEOHEIGHT;
    rect.x = 0;//(int)((1. + .5 * sin(0.03 * c->n)) * (WIDTH - VIDEOWIDTH) / 2);
    rect.y = 0;//(int)((1. + .5 * cos(0.03 * c->n)) * (HEIGHT - VIDEOHEIGHT) / 2);

    //SDL_SetRenderDrawColor(c->renderer, 0, 80, 0, 255);
    SDL_RenderClear(c->renderer);
    SDL_RenderCopy(c->renderer, c->texture, nullptr, &rect);
    SDL_RenderPresent(c->renderer);
}

static void quit(int c) {
    SDL_Quit();
    exit(c);
}

class SDL2VLC_SHARED_EXPORT Video
{
public:
    Video();
    ~Video();
    int run();
private:
    SDL_Renderer* createRenderer();
    void playLoop(Context* c, libvlc_media_player_t *mp);
    void createPlayerAndPlay(libvlc_instance_t* libvlc, std::string path, Context* context);
};

#endif // VIDEO_H
