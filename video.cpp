#include "video.h"

Video::Video(int width, int height)
{
    videoWidth = width;
    videoHeight = height;
}

Video::~Video()
{

}

int Video::play(std::string path, SDL_Renderer *renderer)
{
    Context context;
    context.renderer = renderer;

    context.texture = SDL_CreateTexture(
            context.renderer,
            SDL_PIXELFORMAT_BGR565, SDL_TEXTUREACCESS_STREAMING,
            videoWidth, videoHeight);
    if (!context.texture)
    {
        fprintf(stderr, "Couldn't create texture: %s\n", SDL_GetError());
        return 5;
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

    return EXIT_SUCCESS;
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


    printf("W: %s, H: %s\n", libvlc_video_get_width(mp), libvlc_video_get_height(mp));
    libvlc_video_set_format(mp, "RV16", videoWidth, videoHeight, videoWidth*2);
    libvlc_media_player_play(mp);

    playLoop(context, mp);

    // Stop stream and clean up libVLC.
    libvlc_media_player_stop(mp);
    libvlc_media_player_release(mp);
    libvlc_release(libvlc);

    // Close window and clean up libSDL.
    //delete context;//it isn't a pointer, so it'll clean itself
}

