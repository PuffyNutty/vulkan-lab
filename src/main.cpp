#define DEBUG_MODE

#include <cstdlib>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <stdexcept>
#include <vulkan/vulkan.h>


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}


void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, debugMessenger, pAllocator);
    }
}


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
        _setUpDebugMessenger();
        _pickVisibleDevice();
    }

    void _createVulkanInstance()
    {
        if (_enableValidationLayers && !_checkValidationLayerSupport()) {
            throw std::runtime_error("No validation layers are supported for debugging.\n");
        }

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

        std::vector<const char*> requiredExtensions = _getRequiredExtensions();
        createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        createInfo.enabledExtensionCount = uint32_t(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
        if (_enableValidationLayers) {
            createInfo.enabledLayerCount = uint32_t(_validationLayerNames.size());
            createInfo.ppEnabledLayerNames = _validationLayerNames.data();

            _populateDebugMessengerCreateInfo(&debugMessengerCreateInfo);
            createInfo.pNext = &debugMessengerCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        VkResult instanceCreateResult = vkCreateInstance(&createInfo, nullptr, &_vulkanInstance);
        if (instanceCreateResult != VK_SUCCESS) {
            std::cout << instanceCreateResult << "\n";
            throw std::runtime_error("Failed to create Vulkan Instance");
        }
    }

    bool _checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        
        for (const char* pLayerName : _validationLayerNames)
        {
            bool layerFound = false;
            for (const VkLayerProperties& layerProperties : availableLayers)
            {
                if (std::strcmp(pLayerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }
        return true;
    }


    std::vector<const char*> _getRequiredExtensions()
    {
        uint32_t sdlExtensionCount = 0;
        char const* const* ppSDLExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);
        std::vector<const char*> requiredExtensions(ppSDLExtensions, ppSDLExtensions + sdlExtensionCount);
        if (_enableValidationLayers) {
            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        return requiredExtensions;
    }


    static VKAPI_ATTR VkBool32 VKAPI_CALL _debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    )
    {
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            std::cerr << "Error/warning in validation layer: " << pCallbackData->pMessage << "\n";
        }
        return VK_FALSE;
    }


    void _populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* pCreateInfoToDefine) {
        *pCreateInfoToDefine = {};
        pCreateInfoToDefine->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        pCreateInfoToDefine->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        pCreateInfoToDefine->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        pCreateInfoToDefine->pfnUserCallback = _debugCallback;
    }


    void _setUpDebugMessenger()
    {
        if (!_enableValidationLayers) {
            return;
        }
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        _populateDebugMessengerCreateInfo(&createInfo);
        createInfo.pUserData = nullptr;

        if (CreateDebugUtilsMessengerEXT(_vulkanInstance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("Error when creating debug messenger.\n");
        }
    }


    void _pickVisibleDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(_vulkanInstance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPU/s with Vulkan support.\n");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(_vulkanInstance, &deviceCount, devices.data());

        for (const auto& device : devices)
        {
            if (_isDeviceSuitable(device)) {
                _physicalDevice = device;
                break;
            }
        }

        if (_physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to define physical device.\n");
        }
    }


    bool _isDeviceSuitable(VkPhysicalDevice physicalDevice)
    {
        return true;
        /*
        VkPhysicalDeviceProperties deviceProperties{};
        VkPhysicalDeviceFeatures deviceFeatures{};
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
        std::cout << "device name: " << deviceProperties.deviceName << "\n";
        */
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
        DestroyDebugUtilsMessengerEXT(_vulkanInstance, _debugMessenger, nullptr);
        vkDestroyInstance(_vulkanInstance, nullptr);
    }

private:
    bool _isRunning = true;
    const uint32_t _windowWidth = 500;
    const uint32_t _windowHeight = 500;
    SDL_Window* _pSDLWindow;
    VkInstance _vulkanInstance;

    std::vector<const char*> _validationLayerNames = {
        "VK_LAYER_KHRONOS_validation"
    };
#ifdef DEBUG_MODE
    const bool _enableValidationLayers = true;
#else
    const bool _enableValidationLayers = false;
#endif
    VkDebugUtilsMessengerEXT _debugMessenger;

    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
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