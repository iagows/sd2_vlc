#include "context.h"

Context::Context()
{
    this->killRenderer = true;
}

Context::~Context()
{
    // Close window and clean up libSDL.
    SDL_DestroyMutex(this->mutex);
    if(killRenderer) SDL_DestroyRenderer(this->renderer);
    SDL_DestroyTexture(this->texture);
}

void Context::setRenderKillable(bool v)
{
    this->killRenderer = v;
}

