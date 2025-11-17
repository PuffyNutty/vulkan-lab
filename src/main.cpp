#include <iostream>
#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Error when initialising SDL3.\n";
        SDL_Quit();
        return 1;
    }
    SDL_Window* pWindow = SDL_CreateWindow("Hello there", 500, 500, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::cout << "number of supported extensions: " << extensionCount << "\n";
    
    bool isRunning = true;
    SDL_Event sdlEvent;
    while (isRunning) {
        while (SDL_PollEvent(&sdlEvent)) {
            switch (sdlEvent.type)
            {
            case SDL_EVENT_QUIT:
                isRunning = false;
                break;
            
            default:
                break;
            }
        }
        SDL_Delay(16);
    }
    SDL_DestroyWindow(pWindow);
    SDL_Quit();
    return 0;
}