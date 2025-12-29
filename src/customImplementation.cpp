#include <cstring>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>
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


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessengerToDefine)
{
    PFN_vkCreateDebugUtilsMessengerEXT function = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (function == nullptr) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    } else {
        function(instance, pCreateInfo, pAllocator, pDebugMessengerToDefine);
        return VK_SUCCESS;
    }
}


void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT function = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (function == nullptr) {
        return;
    }
    function(instance, debugMessenger, pAllocator);
}


struct QueueFamilyIndices
{
    uint32_t graphicsFamily;
    bool isComplete = false;
};


class VulkanEngine
{
public:
    VulkanEngine()
    {
        VkApplicationInfo appInfo{};
        appInfo.apiVersion = VK_API_VERSION_1_1;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 2);
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 2);
        appInfo.pApplicationName = "Vulkan Lab";
        appInfo.pEngineName = "Vulkan Engine";
        
        _createInstance(&appInfo);
        _createDebugMessenger();
        _pickPhysicalDevice();
    }


    ~VulkanEngine()
    {
        if (_validationLayersEnabled) {
            DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
        }
        vkDestroyInstance(_instance, nullptr);
    }


private:
    void _createInstance(VkApplicationInfo* pAppInfo)
    {
        if (_validationLayersEnabled and !_validationLayersAreSupported()) {
            std::cout << "validation layers requested, but none are supported.\n";
            return;
        }

        uint32_t supportedExtensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);
        std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions.data());

        std::vector<const char*> requiredExtensionNames = _getRequiredExtensionNames();

        std::cout << "Supported Extensions:\n";
        for (const VkExtensionProperties& extension : supportedExtensions) {
            std::cout << "\t" << extension.extensionName << "\n";
        }
        std::cout << "\nRequired Extensions:\n";
        for (const char* const& pName : requiredExtensionNames) {;
            std::cout << "\t" << pName << "\n";
        }

        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = pAppInfo;
        instanceCreateInfo.enabledExtensionCount = requiredExtensionNames.size();
        instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        instanceCreateInfo.ppEnabledExtensionNames = requiredExtensionNames.data();

        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
        if (_validationLayersEnabled) {
            instanceCreateInfo.enabledLayerCount = _validationLayerNames.size();
            instanceCreateInfo.ppEnabledLayerNames = _validationLayerNames.data();

            debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
            debugMessengerCreateInfo.pfnUserCallback = _debugCallback;
            debugMessengerCreateInfo.pUserData = nullptr;

            instanceCreateInfo.pNext = &debugMessengerCreateInfo;
        } else {
            instanceCreateInfo.enabledLayerCount = 0;
        }
        
        VkResult instanceCreateResult = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);
        if (instanceCreateResult != VK_SUCCESS) {
            std::cerr << "Failed to create instance. Error code: " << instanceCreateResult << "\n";
        }
        std::cout << "instance created successfully.\n";
    }


    bool _validationLayersAreSupported()
    {
        uint32_t supportedLayerCount = 0;
        vkEnumerateInstanceLayerProperties(&supportedLayerCount, nullptr);
        std::vector<VkLayerProperties> supportedLayers(supportedLayerCount);
        vkEnumerateInstanceLayerProperties(&supportedLayerCount, supportedLayers.data());

        for (const char* const& requestedLayerName : _validationLayerNames) {
            for (const VkLayerProperties& layer : supportedLayers) {
                if (std::strcmp(requestedLayerName, layer.layerName) == 0) {
                    return true;
                }
            }
        }
        return false;
    }


    std::vector<const char*> _getRequiredExtensionNames()
    {
        uint32_t SDLExtensionCount = 0;
        const char* const* ppSDLExtensionNames = SDL_Vulkan_GetInstanceExtensions(&SDLExtensionCount);
        std::vector<const char*> requiredExtensionNames(ppSDLExtensionNames, ppSDLExtensionNames + SDLExtensionCount);

        if (_validationLayersEnabled) {
            requiredExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return requiredExtensionNames;
    }


    static VKAPI_ATTR VkBool32 VKAPI_CALL _debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        std::cerr << "Validation Layer says: " << pCallbackData->pMessage << "\n";
        return VK_FALSE;
    }


    void _createDebugMessenger()
    {
        if (!_validationLayersEnabled) {
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
        debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debugMessengerCreateInfo.pfnUserCallback = _debugCallback;
        debugMessengerCreateInfo.pUserData = nullptr;

        VkResult debugMessengerCreateResult = CreateDebugUtilsMessengerEXT(_instance, &debugMessengerCreateInfo, nullptr, &_debugMessenger);
        if (debugMessengerCreateResult != VK_SUCCESS) {
            std::cerr << "Failed to create debug messenger. Error code: " << debugMessengerCreateResult << "\n";
            return;
        }
        std::cout << "Debug messenger created successfully.\n";
    }


    void _pickPhysicalDevice()
    {
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            std::cerr << "Failed to find GPUs with Vulkan support\n";
            return;
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

        for (const VkPhysicalDevice& device : devices) {
            if (_physicalDeviceIsSuitable(device)) {
                _physicalDevice = device;
            }
        }

        if (_physicalDevice == VK_NULL_HANDLE) {
            std::cerr << "Failed to find a suitable GPU\n";
            return;
        }
    }


    bool _physicalDeviceIsSuitable(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties2 properties;
        properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        vkGetPhysicalDeviceProperties2(device, &properties);
        QueueFamilyIndices indices = _getQueueFamilies(device);
        if (!indices.isComplete) {
            return false;
        }
        std::cout << "The device " << properties.properties.deviceName << " is suitable.\n";
        return true;
    }


    QueueFamilyIndices _getQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices{};

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        uint32_t i = 0;
        for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
                indices.isComplete = true;
                return indices;
            }
            i++;
        }
        std::cerr << "Unable to find suitable queue families\n";
        return indices;
    }


private:
    VkInstance _instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    std::vector<const char*> _validationLayerNames = {
        "VK_LAYER_KHRONOS_validation"
    };
    const bool _validationLayersEnabled = true;
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