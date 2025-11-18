#include <cstdlib>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <stdexcept>
#include <vulkan/vulkan.h>


class VulkanApp
{
public:
    void run()
    {
        _initVulkan();
        _mainLoop();
        _cleanup();
    }

private:
    void _initVulkan()
    {
        _pSDLWindow = SDL_CreateWindow("Vulkan Lab", _windowWidth, _windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
        _createVulkanInstance();
    }

    void _createVulkanInstance()
    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        uint32_t SDLExtensionCount = 0;
        char const* const* ppSDLExtensions;
        ppSDLExtensions = SDL_Vulkan_GetInstanceExtensions(&SDLExtensionCount);

        std::vector<char const*> requiredExtensions;
        for (uint32_t i = 0; i < SDLExtensionCount; i++)
        {
            requiredExtensions.push_back(ppSDLExtensions[i]);
        }

        requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        createInfo.enabledExtensionCount = uint32_t(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        createInfo.enabledLayerCount = 0;

        VkResult instanceCreateResult = vkCreateInstance(&createInfo, nullptr, &_vulkanInstance);
        if (instanceCreateResult != VK_SUCCESS) {
            std::cout << instanceCreateResult << "\n";
            throw std::runtime_error("Failed to create Vulkan Instance");
        }
        vkDestroyInstance(_vulkanInstance, nullptr);
    }

    void _mainLoop()
    {
        SDL_Event event;
        while (_isRunning) {
            while (SDL_PollEvent(&event))
            {
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
        SDL_DestroyWindow(_pSDLWindow);
    }

private:
    bool _isRunning = true;
    const uint32_t _windowWidth = 500;
    const uint32_t _windowHeight = 500;
    SDL_Window* _pSDLWindow;
    VkInstance _vulkanInstance;
};


int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Error when initialising SDL3.\n";
        SDL_Quit();
        return EXIT_FAILURE;
    }
    VulkanApp app;
    app.run();
    SDL_Quit();
    return EXIT_SUCCESS;
}