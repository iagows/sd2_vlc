#include "video.h"

Video::Video()
{

}

Video::~Video()
{

}

int Video::run()
{
    std::string path = "a.mp4";

    // Initialise libSDL.
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Could not initialize SDL: %s.\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    Context context;
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

    // If you don't have this variable set you must have plugins directory with the executable or libvlc_new() will not work!
    //printf("VLC_PLUGIN_PATH=%s\n", getenv("VLC_PLUGIN_PATH"));

    // Initialise libVLC.
    libvlc_instance_t *libvlc;
    char const *vlc_argv[] = {

        //"--no-audio", // Don't play audio.
        //"--no-xlib", // Don't use Xlib.

        // Apply a video filter.
        //"--video-filter", "sepia", "--sepia-intensity=200"
    };
    int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);

    libvlc = libvlc_new(vlc_argc, vlc_argv);
    if(libvlc == nullptr)
    {
        printf("LibVLC initialization failure.\n");
        return EXIT_FAILURE;
    }

    createPlayerAndPlay(libvlc, path, &context);

    quit(EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

SDL_Renderer *Video::createRenderer()
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

void Video::playLoop(Context *c, libvlc_media_player_t *mp)
{
    SDL_Event event;
    bool done = false;
    int action = 0;
    bool pause = false;

    bool previousPause = false;

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

        if(!pause)
        {
            c->n++;
        }

        if(!previousPause && pause)
        {
            libvlc_media_player_pause(mp);
            previousPause = true;
        }
        else if(previousPause && !pause)
        {
            libvlc_media_player_play(mp);
            previousPause = false;
        }

        SDL_Delay(1000/10);
    }
}

void Video::createPlayerAndPlay(libvlc_instance_t *libvlc, std::string path, Context *context)
{
    libvlc_media_t *m;
    libvlc_media_player_t *mp;

    m = libvlc_media_new_path(libvlc, path.c_str());
    mp = libvlc_media_player_new_from_media(m);
    libvlc_media_release(m);

    libvlc_video_set_callbacks(mp, lock, unlock, display, context);
    libvlc_video_set_format(mp, "RV16", VIDEOWIDTH, VIDEOHEIGHT, VIDEOWIDTH*2);
    libvlc_media_player_play(mp);

    playLoop(context, mp);

    // Stop stream and clean up libVLC.
    libvlc_media_player_stop(mp);
    libvlc_media_player_release(mp);
    libvlc_release(libvlc);

    // Close window and clean up libSDL.
    //delete context;//it isn't a pointer, so it'll clean itself
}

