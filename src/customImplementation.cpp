#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>


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


class VulkanEngine
{
public:
    VulkanEngine()
    {
        VkApplicationInfo appInfo{};
        appInfo.apiVersion = VK_API_VERSION_1_0;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pApplicationName = "Vulkan Lab";
        appInfo.pEngineName = "Vulkan Engine";
        
        _createInstance(&appInfo);
    }


    ~VulkanEngine()
    {
        vkDestroyInstance(_instance, nullptr);
    }


private:
    void _createInstance(VkApplicationInfo* pAppInfo)
    {
        uint32_t supportedExtensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);
        std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions.data());

        uint32_t requiredExtensionCount = 0;
        const char* const* ppRequiredExtensionNames = SDL_Vulkan_GetInstanceExtensions(&requiredExtensionCount);

        std::cout << "Supported Extensions:\n";
        for (const VkExtensionProperties& extension : supportedExtensions) {
            std::cout << "\t" << extension.extensionName << "\n";
        }
        std::cout << "\nRequired Extensions:\n";
        for (uint32_t i = 0; i < requiredExtensionCount; i++) {
            const char* pName = ppRequiredExtensionNames[i];
            std::cout << "\t" << pName << "\n";
        }

        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = pAppInfo;
        instanceCreateInfo.enabledExtensionCount = requiredExtensionCount;
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        instanceCreateInfo.ppEnabledExtensionNames = ppRequiredExtensionNames;
        
        VkResult instanceCreateResult = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);
        if (instanceCreateResult != VK_SUCCESS) {
            std::cout << "Failed to create instance. Error code: " << instanceCreateResult << "\n";
        }
        std::cout << "instance created successfully.\n";
    }


private:
    VkInstance _instance;
};


int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed to initialse SDL\n";
        return EXIT_FAILURE;
    }
    Window window("Vulkan lab custom implementation", 500, 500);
    VulkanEngine vulkanEngine;
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