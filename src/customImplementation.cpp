#include <iostream>
#include <SDL3/SDL.h>


typedef uint32_t Pixels;


class Window
{
public:
    Window(const char* pTitle, Pixels width, Pixels height) 
    : _pTitle(pTitle), _width(width), _height(height)
    {
        _pSDLWindow = SDL_CreateWindow(_pTitle, _width, _height, SDL_WINDOW_VULKAN);
        if (!_pSDLWindow) {
            std::cerr << "Failed to create SDL window.\n";
        } 
    }


    ~Window()
    {
        SDL_DestroyWindow(_pSDLWindow);
    }


    bool isActive()
    {
        return _isActive;
    }


    void close()
    {
        _isActive = false;
    }


private:
    bool _isActive = true;
    const char* _pTitle;
    const Pixels _width;
    const Pixels _height;
    SDL_Window* _pSDLWindow = nullptr;
};


int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed to initialse SDL\n";
        return EXIT_FAILURE;
    }
    Window window("Vulkan lab custom implementation", 500, 500);
    SDL_Event event;
    while (window.isActive()) {
        while (SDL_PollEvent(&event)) {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                window.close();
                break;
            
            default:
                break;
            }
        }
        SDL_Delay(16);
    }
    return EXIT_SUCCESS;
}