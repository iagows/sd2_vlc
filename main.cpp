//https://wiki.videolan.org/LibVLC_Users/
//https://wiki.videolan.org/LibVLC_SampleCode_SDL/

//SDL 2.0
//This version works with LibVLC 1.1.1 or later and SDL 2.0.
// libSDL and libVLC sample code.
// License: [http://en.wikipedia.org/wiki/WTFPL WTFPL]

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

#ifdef __MINGW32__
#undef main
#endif

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

SDL_Renderer * createRenderer()
{
    // Create SDL graphics objects.
    SDL_Window * window = SDL_CreateWindow(
            "Mimimi",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            VIDEOWIDTH, VIDEOHEIGHT,
            SDL_WINDOW_SHOWN/*|SDL_WINDOW_RESIZABLE*/);
    if (!window)
    {
        fprintf(stderr, "Couldn't create window: %s\n", SDL_GetError());
        quit(3);
    }

    SDL_Renderer* ren = SDL_CreateRenderer(window, -1, 0);
    if (!ren)
    {
        fprintf(stderr, "Couldn't create renderer: %s\n", SDL_GetError());
        quit(4);
    }

    return ren;
}

void stayOnLoop(Context* c)
{
    SDL_Event event;
    bool done = false;
    int action = 0;
    bool pause = false;

    // Main loop.
    while(!done)
    {
        action = 0;

        // Keys: enter (fullscreen), space (pause), escape (quit).
        while( SDL_PollEvent( &event )) {

            switch(event.type) {
                case SDL_QUIT:
                    done = true;
                    break;
                case SDL_KEYDOWN:
                    action = event.key.keysym.sym;
                    break;
            }
        }

        switch(action) {
            case SDLK_ESCAPE:
            case SDLK_q:
                done = true;
                break;
            case SDLK_SPACE:
                printf("Pause toggle.\n");
                pause = !pause;
                break;
        }

        if(!pause) { c->n++; }

        SDL_Delay(1000/10);
    }
}

int main()
{
    std::string path = "a.mp4";
    libvlc_instance_t *libvlc;
    libvlc_media_t *m;
    libvlc_media_player_t *mp;
    char const *vlc_argv[] = {

        //"--no-audio"//, // Don't play audio.
        //"--no-xlib"//, // Don't use Xlib.

        // Apply a video filter.
        //"--video-filter", "sepia", "--sepia-intensity=200"
    };
    int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);

    Context context;

    // Initialise libSDL.
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Could not initialize SDL: %s.\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    context.renderer = createRenderer();

    context.texture = SDL_CreateTexture(
            context.renderer,
            SDL_PIXELFORMAT_BGR565, SDL_TEXTUREACCESS_STREAMING,
            VIDEOWIDTH, VIDEOHEIGHT);
    if (!context.texture)
    {
        fprintf(stderr, "Couldn't create texture: %s\n", SDL_GetError());
        quit(5);
    }

    context.mutex = SDL_CreateMutex();

    // If you don't have this variable set you must have plugins directory
    // with the executable or libvlc_new() will not work!
#ifdef DEBUG
    printf("VLC_PLUGIN_PATH=%s\n", getenv("VLC_PLUGIN_PATH"));
#endif

    // Initialise libVLC.
    libvlc = libvlc_new(vlc_argc, vlc_argv);
    if(libvlc == nullptr)
    {
        printf("LibVLC initialization failure.\n");
        return EXIT_FAILURE;
    }

    m = libvlc_media_new_path(libvlc, path.c_str());//m = libvlc_media_new_path(libvlc, argv[1]);
    mp = libvlc_media_player_new_from_media(m);
    libvlc_media_release(m);

    libvlc_video_set_callbacks(mp, lock, unlock, display, &context);
    libvlc_video_set_format(mp, "RV16", VIDEOWIDTH, VIDEOHEIGHT, VIDEOWIDTH*2);
    libvlc_media_player_play(mp);

    stayOnLoop(&context);

    // Stop stream and clean up libVLC.
    libvlc_media_player_stop(mp);
    libvlc_media_player_release(mp);
    libvlc_release(libvlc);

    // Close window and clean up libSDL.
    //delete context;//it isn't a pointer, so it'll clean itself

    quit(0);

    return 0;
}
