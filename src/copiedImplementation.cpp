#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>


class HelloTriangleApplication
{
public:
    void run()
    {
        _initWindow();
        _initVulkan();
        _mainLoop();
        _cleanup();
    }


private:
    void _initWindow()
    {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error("Could not initialise SDL");
        }

        _pWindow = SDL_CreateWindow("Vulkan lab copied implementation", 500, 500, SDL_WINDOW_VULKAN);
        if (!_pWindow) {
            throw std::runtime_error("Could not create SDL window");
        }
    }


    void _initVulkan()
    {

    }


    void _mainLoop()
    {
        SDL_Event event;
        while (_isRunning) {
            while (SDL_PollEvent(&event)) {
                switch (event.type)
                {
                case SDL_EVENT_QUIT:
                    _isRunning = false;
                    break;
                
                default:
                    break;
                }
            }
            SDL_Delay(16);
        }

    }


    void _cleanup()
    {
        SDL_DestroyWindow(_pWindow);
        SDL_Quit();
    }


private:
    bool _isRunning = true;
    const uint32_t _windowWidth = 500;
    const uint32_t _windowHeight = 500;
    SDL_Window* _pWindow = nullptr;
};


int main()
{
    HelloTriangleApplication app;
    try {
        app.run();
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}