// --- GL Includes ---
#include "app.hpp"

// --- STL Includes ---
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <string>
#include <algorithm>
#include <sstream>
#include <type_traits>


VkResult createDebugUtilsMessenger(VkInstance instance,
                                   const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator,
                                   VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto function = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (function != nullptr) {
        return function(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}


void destroyDebugUtilsMessenger(VkInstance instance,
                                VkDebugUtilsMessengerEXT debugMessenger,
                                const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


Application::Application()
{
    this->initVulkan();
    this->initWindow();
}


Application::~Application()
{
    if (_enableValidationLayers)
        destroyDebugUtilsMessenger(_vulkanInstance, _debugMessenger, nullptr);
    vkDestroyInstance(_vulkanInstance, nullptr);
    glfwDestroyWindow(_p_window);
    glfwTerminate();
}


void Application::run()
{
    //this->getExtensions();
    this->mainLoop();
}


void Application::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // <== GLFW without OpenGL

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // <== todo, disable window resizing for now
    _p_window = glfwCreateWindow(_windowWidth, _windowHeight, "vktutorial", nullptr, nullptr);
}

void Application::initVulkan()
{
    this->createVkInstance();
    this->initDebugMessenger();
}


template <concepts::Iterator ItOutput>
void Application::getExtensions(ItOutput it_out)
{
    uint32_t numberOfExtensions = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &numberOfExtensions, nullptr);
    std::vector<VkExtensionProperties> extensions(numberOfExtensions);
    vkEnumerateInstanceExtensionProperties(nullptr, &numberOfExtensions, extensions.data());

    std::cout << "Available vulkan extensions:\n";
    for (const auto& r_extension : extensions)
        *it_out++ = r_extension;
}

template <concepts::Iterator OutputIt>
void Application::getRequiredExtensions(OutputIt it)
{
    uint32_t numberOfGLFWExtensions = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&numberOfGLFWExtensions);
    for (uint32_t i=0; i<numberOfGLFWExtensions; ++i)
        *it++ = *glfwExtensions++;

    if (_enableValidationLayers) {
        *it++ = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }
}

template <concepts::Iterator<std::string> TInputIt, concepts::Iterator TItOutput>
void Application::getSupportedLayers(TInputIt begin, TInputIt end, TItOutput it_out)
{
    uint32_t numberOfLayers;
    vkEnumerateInstanceLayerProperties(&numberOfLayers, nullptr);
    std::vector<VkLayerProperties> availableLayers(numberOfLayers);
    vkEnumerateInstanceLayerProperties(&numberOfLayers, availableLayers.data());

    for (; begin!=end; ++begin) {
        if (std::any_of(availableLayers.begin(),
                        availableLayers.end(),
                        [begin](const auto& r_name)
                            {std::cout << r_name.layerName << std::endl; return r_name.layerName == *begin;})) {
            *it_out++ = *begin;
        } // if validationLayer in availableLayers
    } // for layerName in validationLayers
}

VKAPI_ATTR VkBool32 VKAPI_CALL Application::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                          VkDebugUtilsMessageTypeFlagsEXT type,
                                                          const VkDebugUtilsMessengerCallbackDataEXT* p_data,
                                                          void* p_userData)
{
    std::cerr << "Validation layer";

    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        std::cerr << " [VERBOSE]";
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        std::cerr << " [INFO]";
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        std::cerr << " [WARNING]";
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        std::cerr << " [ERROR]";

    if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
        std::cerr << " (GENERAL)";
    if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        std::cerr << " (VALIDATION)";
    if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
        std::cerr << " (PERFORMANCE)";

    std::cerr << ": " << p_data->pMessage << std::endl;
    return VK_FALSE;
}

void Application::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = Application::debugCallback;
}

void Application::initDebugMessenger()
{
    #ifndef NDEBUG
    VkDebugUtilsMessengerCreateInfoEXT createInfo {};
    this->populateDebugMessengerCreateInfo(createInfo);
    createInfo.pUserData = nullptr;

    if (createDebugUtilsMessenger(_vulkanInstance,
                                    &createInfo,
                                    nullptr,
                                    &_debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to set up debug callback for vulkan");
    }
    #endif
}

void Application::createVkInstance()
{
    // Optional info struct for optimizin vulkan calls
    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "vktutorial";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "none";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Specify required global extensions and validation layers
    VkInstanceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<std::string> extensionNames;
    this->getRequiredExtensions(std::back_inserter(extensionNames));

    std::vector<const char*> cStrings;
    cStrings.reserve(extensionNames.size());
    for (const auto& r_name : extensionNames)
        cStrings.push_back(r_name.c_str());
    createInfo.enabledExtensionCount = cStrings.size();
    createInfo.ppEnabledExtensionNames = cStrings.data();

    // Check validation layer support
    std::vector<std::string> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    std::sort(validationLayers.begin(), validationLayers.end());
    std::vector<const char*> validationLayerCStrings(validationLayers.size());
    std::transform(validationLayers.begin(), validationLayers.end(), validationLayerCStrings.begin(), [](const auto& r_name) {return r_name.c_str();});

    if (_enableValidationLayers) {
        std::vector<std::string> supportedValidationLayers;
        this->getSupportedLayers(validationLayers.begin(), validationLayers.end(), std::back_inserter(supportedValidationLayers));

        // Error if not all validation layers are supported
        if (validationLayers.size() != supportedValidationLayers.size()) {
            std::vector<std::string> unsupportedLayers;
            std::sort(supportedValidationLayers.begin(), supportedValidationLayers.end());
            std::set_difference(validationLayers.begin(),
                                validationLayers.end(),
                                supportedValidationLayers.begin(),
                                supportedValidationLayers.end(),
                                std::back_inserter(unsupportedLayers));
            std::stringstream message;
            message << "Validation layers requested, but not available:";
            for (const auto& r_layerName : unsupportedLayers) {
                message << ' ' << r_layerName;
            }
            throw std::runtime_error(message.str());
        } // if validation layers not supported

        createInfo.enabledLayerCount = validationLayerCStrings.size();
        createInfo.ppEnabledLayerNames = validationLayerCStrings.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    // Set up debugging for vkCreateInstance
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
    if (_enableValidationLayers) {
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.pNext = nullptr;
    }

    // Create vulkan instance
    if (vkCreateInstance(&createInfo, nullptr, &_vulkanInstance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create vulkan instance");
    }
}

void Application::mainLoop()
{
    while (!glfwWindowShouldClose(_p_window)) {
        glfwPollEvents();
        break;
    } // while not window_should_close
}
