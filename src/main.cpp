#define DEBUG_MODE

#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>


VkResult CreateDebugUtilsMessenger(VkInstance_T* pVkInstance, const VkDebugUtilsMessengerCreateInfoEXT* pDebugMessengerCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT_T** ppDebugMessengerToDefine)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(pVkInstance, "vkCreateDebugUtilsMessengerEXT");
    if (!func) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    return func(pVkInstance, pDebugMessengerCreateInfo, pAllocator, ppDebugMessengerToDefine);
}


void DestroyDebugUtilsMessenger(VkInstance_T* pVkInstance, VkDebugUtilsMessengerEXT_T* pDebugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(pVkInstance, "vkDestroyDebugUtilsMessengerEXT");
    if (!func) {
        return;
    }
    return func(pVkInstance, pDebugMessenger, pAllocator);
}


class HelloTriangleApp
{
public:
    HelloTriangleApp()
    {
        _initWindow();
        _initVulkan();
        _setupDebugMessenger();
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
        if (_enableValidationLayers and !_validationLayersAreSupported()) {
            throw std::runtime_error("Validation layers are requested, but none are available!");
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
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        if (_enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayerNames.size());
            createInfo.ppEnabledLayerNames = _validationLayerNames.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        uint32_t supportedExtensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);
        std::vector<VkExtensionProperties> supportedExtensionProperties(supportedExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensionProperties.data());
        std::cout << "\nAll Supported Extensions:\n";
        for (const VkExtensionProperties& supportedExtension : supportedExtensionProperties) {
            std::cout << "\t" << supportedExtension.extensionName << "\n";
        }

        std::vector<const char*> requiredExtensionNames = _getNamesOfRequiredExtensions();
        std::cout << "\nAll Required Extensions:\n";
        for (const char* pExtensionName : requiredExtensionNames) {
            std::cout << "\t" << pExtensionName << "\n";
        }

        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensionNames.size());
        createInfo.ppEnabledExtensionNames = requiredExtensionNames.data();

        if (_enableValidationLayers) {
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            _populateDebugMessengerCreateInfo(&debugCreateInfo);
            createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayerNames.size());
            createInfo.ppEnabledLayerNames = _validationLayerNames.data();
            createInfo.pNext = &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &_pVkInstance);
        if (result != VK_SUCCESS) {
            std::string errorMessage = "Failed to define VkInstance! Error code: ";
            errorMessage += std::to_string(result);
            throw std::runtime_error(errorMessage);
        }
    }


    bool _validationLayersAreSupported()
    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* pLayerName : _validationLayerNames) {
            bool layerFound = false;
            for (const VkLayerProperties& layerProperties : availableLayers) {
                if (strcmp(pLayerName, layerProperties.layerName) == 0) {
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


    std::vector<const char*> _getNamesOfRequiredExtensions()
    {
        uint32_t SDLExtensionCount = 0;
        // An immutable array of several c-style strings
        const char* const* ppSDLExtensionNames = SDL_Vulkan_GetInstanceExtensions(&SDLExtensionCount);
        std::vector<const char*> requiredExtensionNames;
        for (uint i = 0; i < SDLExtensionCount; i++) {
            requiredExtensionNames.emplace_back(ppSDLExtensionNames[i]);
        }
        if (_enableValidationLayers) {
            requiredExtensionNames.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return requiredExtensionNames;
    }


    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messagetype, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        std::cerr << "Validation Layer: " << pCallbackData->pMessage << "\n";
        return VK_FALSE;
    }


    void _setupDebugMessenger()
    {
        if (!_enableValidationLayers) {
            return;
        }
        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
        _populateDebugMessengerCreateInfo(&debugMessengerCreateInfo);

        if (CreateDebugUtilsMessenger(_pVkInstance, &debugMessengerCreateInfo, nullptr, &_pDebugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("Could not create Debug Messenger\n");
        }
    }


    void _populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* pCreateInfoToDefine)
    {
        pCreateInfoToDefine->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        pCreateInfoToDefine->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        pCreateInfoToDefine->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        pCreateInfoToDefine->pfnUserCallback = debugCallback;
        pCreateInfoToDefine->pUserData = nullptr;
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
        //DestroyDebugUtilsMessenger(_pVkInstance, _pDebugMessenger, nullptr);
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
    VkDebugUtilsMessengerEXT_T* _pDebugMessenger = nullptr;
    const std::vector<const char*> _validationLayerNames = {
        "VK_LAYER_KHRONOS_validation"
    };
    #ifndef DEBUG_MODE
        const bool _enableValidationLayers = false;
    #else
        const bool _enableValidationLayers = true;
    #endif
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