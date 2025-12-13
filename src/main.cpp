#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>


class HelloTriangleApp
{
public:
    HelloTriangleApp()
    {
        _initWindow();
        _initVulkan();
    }


    ~HelloTriangleApp()
    {
        _cleanup();
    }


    void start()
    {
        _isRunning = true;
        _mainLoop();
    }


private:
    void _initWindow()
    {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error("Failed to initalise SDL3");
        }

        _pWindow = SDL_CreateWindow("Vulkan Lab", 500, 500, SDL_WINDOW_VULKAN);
        if (!_pWindow) {
            throw std::runtime_error("Failed to create SDL window");
        }
    }


    void _initVulkan()
    {
        _createInstance();
    }


    void _createInstance()
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

        uint32_t supportedExtensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);
        std::vector<VkExtensionProperties> supportedExtensionProperties(supportedExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensionProperties.data());
        std::cout << "\nAll Supported Extensions:\n";
        for (const VkExtensionProperties& supportedExtension : supportedExtensionProperties) {
            std::cout << "\t" << supportedExtension.extensionName << "\n";
        }

        uint32_t requiredExtensionCount = 0;
        // An immutable array of several c-style strings
        const char* const* ppRequiredExtensionNames = SDL_Vulkan_GetInstanceExtensions(&requiredExtensionCount);
        std::cout << "\nAll Required Extensions:\n";
        for (uint32_t i = 0; i < requiredExtensionCount; i++) {
            std::cout << "\t" << ppRequiredExtensionNames[i] << "\n";
        }

        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        createInfo.enabledExtensionCount = requiredExtensionCount;
        createInfo.ppEnabledExtensionNames = ppRequiredExtensionNames;
        createInfo.enabledLayerCount = 0;

        VkResult result = vkCreateInstance(&createInfo, nullptr, &_pVkInstance);
        if (result != VK_SUCCESS) {
            std::string errorMessage = "Failed to define VkInstance! Error code: ";
            errorMessage += std::to_string(result);
            throw std::runtime_error(errorMessage);
        }
    }


    void _mainLoop()
    {
        while (_isRunning) {
            while (SDL_PollEvent(&_event)) {
                switch (_event.type)
                {
                case SDL_EVENT_QUIT:
                    _isRunning = false;
                    break;
                
                default:
                    break;
                }
                SDL_Delay(16);
            }
        }
    }


    void _cleanup()
    {
        vkDestroyInstance(_pVkInstance, nullptr);
        SDL_DestroyWindow(_pWindow);
        SDL_Quit();
    }


private:
    bool _isRunning = false;
    SDL_Event _event;


private:
    SDL_Window* _pWindow = nullptr;
    VkInstance_T* _pVkInstance = nullptr;
};


int main()
{
    try {
        HelloTriangleApp app;
        app.start();
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}