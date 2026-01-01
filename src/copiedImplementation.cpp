#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <optional>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <set>
#include <vulkan/vulkan.hpp>

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}


void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, messenger, pAllocator);
    }
}


struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};


struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


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
        _createInstance();
        _setupDebugMessenger();
        _createSurface();
        _pickPhysicalDevice();
        _createLogicalDevice();
        _createSwapChain();
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
        vkDestroySwapchainKHR(_device, _swapchain, nullptr);
        vkDestroyDevice(_device, nullptr);
        SDL_Vulkan_DestroySurface(_instance, _surface, nullptr);
        if (_enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
        }
        vkDestroyInstance(_instance, nullptr);
        SDL_DestroyWindow(_pWindow);
        SDL_Quit();
    }


    void _createInstance()
    {
        if (_enableValidationLayers && !_checkValidationLayerSupport()) {
            throw std::runtime_error("Validation layers requested, but none are supported");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.apiVersion = VK_API_VERSION_1_0;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 2);
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 2);
        appInfo.pApplicationName = "Vulkan lab";
        appInfo.pEngineName = "No engine";

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        std::vector<const char*> requiredExtensions = _getRequiredExtensions();
        createInfo.enabledExtensionCount = requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
        if (_enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
            createInfo.ppEnabledLayerNames = _validationLayers.data();
            _populateDebugMessengerCreateInfo(&debugMessengerCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugMessengerCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create vulkan instance");
        }

        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        std::cout << "Available extensions:\n";
        for (VkExtensionProperties& extension : extensions) {
            std::cout << "\t" << extension.extensionName << "\n";
        }
    }


    bool _checkValidationLayerSupport()
    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* pLayerName : _validationLayers) {
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


    std::vector<const char*> _getRequiredExtensions()
    {
        uint32_t SDLExtensionCount = 0;
        const char* const* ppSDLExtensionNames = SDL_Vulkan_GetInstanceExtensions(&SDLExtensionCount);
        std::vector<const char*> requiredExtensions(ppSDLExtensionNames, ppSDLExtensionNames + SDLExtensionCount);
        requiredExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

        if (_enableValidationLayers) {
            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        std::cout << "\nRequired Extensions:\n";
        for (const char* const& extension : requiredExtensions) {
            std::cout << "\t" << extension << "\n";
        }

        return requiredExtensions;
    }


    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ) {
        std::cerr << "Validation Layer Reports: " << pCallbackData->pMessage << "\n";
        return VK_FALSE;
    }


    void _populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo)
    {
        pCreateInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        pCreateInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        pCreateInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        pCreateInfo->pfnUserCallback = debugCallback;
        pCreateInfo->pUserData = nullptr;
    }


    void _setupDebugMessenger()
    {
        if (!_enableValidationLayers) {
            return;
        }
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        _populateDebugMessengerCreateInfo(&createInfo);
        createInfo.pUserData = nullptr;

        if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create DebugUtilsMessenger");
        }
    }


    void _pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs that support Vulkan");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

        for (const VkPhysicalDevice& device : devices) {
            if (_isDeviceSuitable(device)) {
                _physicalDevice = device;
                break;
            }
        }

        if (_physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("You have GPU/s that support Vulkan, though are not suitable for this program.");
        }
    }


    bool _isDeviceSuitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = _findQueueFamilies(device);
        bool extensionsSupported = _checkDeviceExtensionSupport(device);
        bool swapchainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapchainSupport = _querySwapChainSupport(device);
            swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
        }
        return indices.isComplete() && extensionsSupported && swapchainAdequate;
    }


    QueueFamilyIndices _findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        uint32_t i = 0;
        for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);
            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }
            i++;
        }

        return indices;
    }


    void _createLogicalDevice()
    {
        QueueFamilyIndices indices = _findQueueFamilies(_physicalDevice);
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures physicalDeviceFeatures{}; // Initialise everything as VK_FALSE

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pEnabledFeatures = &physicalDeviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(_deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = _deviceExtensions.data();

        if (_enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
            createInfo.ppEnabledLayerNames = _validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }
        VkResult deviceCreateResult = vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device);
        if (deviceCreateResult != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device.\n");
        }

        vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
        vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);

        std::cout << "successfullly created logical device!\n";
    }


    void _createSurface()
    {
        if (!SDL_Vulkan_CreateSurface(_pWindow, _instance, nullptr, &_surface)) {
            throw std::runtime_error("Failed to create window surface.\n");
        }
    }


    bool _checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
        std::set<std::string> requiredExtensions(_deviceExtensions.begin(), _deviceExtensions.end());
        std::cout << "\nAvailable device extensions:\n";
        for (const VkExtensionProperties& extension : availableExtensions) {
            std::cout << "\t" << extension.extensionName << "\n";
            requiredExtensions.erase(extension.extensionName);
        }
        return requiredExtensions.empty();
    }


    SwapChainSupportDetails _querySwapChainSupport(VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }


    VkSurfaceFormatKHR _chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }


    VkPresentModeKHR _chooseSwapPresentMode(std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& presentMode : availablePresentModes) {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return presentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }


    VkExtent2D _chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            SDL_GetWindowSizeInPixels(_pWindow, &width, &height);
            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.height);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return actualExtent;
        }
    }


    void _createSwapChain()
    {
        SwapChainSupportDetails swapchainSupport = _querySwapChainSupport(_physicalDevice);
        VkSurfaceFormatKHR surfaceFormat = _chooseSwapSurfaceFormat(swapchainSupport.formats);
        VkPresentModeKHR presentMode = _chooseSwapPresentMode(swapchainSupport.presentModes);
        VkExtent2D extent = _chooseSwapExtent(swapchainSupport.capabilities);

        uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
        // a maxImageCount of 0 means there is no maximum. So this is 'if (there is a maximum) and (imageCount is greater than it)'
        if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
            imageCount = swapchainSupport.capabilities.maxImageCount;
        }
        
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = _surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = _findQueueFamilies(_physicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // optional.
            createInfo.pQueueFamilyIndices = nullptr; // optional.
        }
        createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        
        VkResult result = vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapchain);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swapchain.\n");
        }

        vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr);
        _swapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, _swapchainImages.data());

        _swapchainImageFormat = surfaceFormat.format;
        _swapchainExtent = extent;

        std::cout << "successfully created swapchain!\n";
    }


private:
    bool _isRunning = true;
    const uint32_t _windowWidth = 500;
    const uint32_t _windowHeight = 500;
    SDL_Window* _pWindow = nullptr;
    VkInstance _instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _device = VK_NULL_HANDLE;
    VkQueue _graphicsQueue = VK_NULL_HANDLE;
    VkQueue _presentQueue = VK_NULL_HANDLE;
    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> _swapchainImages;
    VkFormat _swapchainImageFormat;
    VkExtent2D _swapchainExtent;

    const std::vector<const char*> _validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const std::vector<const char*> _deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        "VK_KHR_portability_subset"
    };
    
#ifdef NDEBUG
    const bool _enableValidationLayers = false;
#else
    const bool _enableValidationLayers = true;
#endif
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


/*
argndm
*/