#include <cstring>
#include <iostream>
#include <optional>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
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

    bool isComplete()
    {
        return graphicsFamily.has_value();
    }
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
        _pickPhysicalDevice();
        _createLogicalDevice();
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
        vkDestroyDevice(_device, nullptr);
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
        return indices.isComplete();
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
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
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures physicalDeviceFeatures{}; // Initialise everything as VK_FALSE

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &physicalDeviceFeatures;

        uint32_t supportedExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &supportedExtensionCount, nullptr);
        std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
        vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &supportedExtensionCount, supportedExtensions.data());
        std::cout << "\nAvailable device extensions:\n";
        for (const VkExtensionProperties& extension : supportedExtensions) {
            std::cout << "\t" << extension.extensionName << "\n";
        }
        const char* ppRequiredExtensions[1] = {"VK_KHR_portability_subset"};
        createInfo.enabledExtensionCount = 1;
        createInfo.ppEnabledExtensionNames = ppRequiredExtensions;

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

        std::cout << "successfullly created logical device!\n";
    }


private:
    bool _isRunning = true;
    const uint32_t _windowWidth = 500;
    const uint32_t _windowHeight = 500;
    SDL_Window* _pWindow = nullptr;
    VkInstance _instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _device = VK_NULL_HANDLE;
    VkQueue _graphicsQueue = VK_NULL_HANDLE;

    const std::vector<const char*> _validationLayers = {
        "VK_LAYER_KHRONOS_validation"
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