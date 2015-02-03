#include "context.h"

Context::Context()
{

}

Context::~Context()
{
    // Close window and clean up libSDL.
    SDL_DestroyMutex(this->mutex);

    SDL_DestroyTexture(this->texture);
}

