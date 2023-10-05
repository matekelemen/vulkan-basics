// --- GL Includes ---
#include "Application.hpp"
#include "VulkanInstance.hpp"
#include "DebugMessenger.hpp"
#include "WindowSurface.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "SwapChain.hpp"

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
#include <optional>


struct Application::Impl
{
    Impl()
        : _p_window(nullptr),
          _p_vulkanInstance(),
          _p_windowSurface(),
          _p_physicalDevice(),
          _p_logicalDevice(),
          _p_swapChain(),
          _p_imageViews()
    {
    }

    ~Impl()
    {
        #ifndef NDEBUG
        _debugMessenger.reset();
        #endif
        _p_imageViews.reset();
        _p_swapChain.reset();
        _p_logicalDevice.reset();
        _p_physicalDevice.reset();
        _p_windowSurface.reset();
        _p_vulkanInstance.reset();
        glfwDestroyWindow(_p_window);
    }

    GLFWwindow* _p_window;

    std::shared_ptr<VulkanInstance> _p_vulkanInstance;

    std::shared_ptr<WindowSurface> _p_windowSurface;

    std::shared_ptr<PhysicalDevice> _p_physicalDevice;

    std::shared_ptr<GraphicsLogicalDevice> _p_logicalDevice;

    std::shared_ptr<SwapChain> _p_swapChain;

    std::shared_ptr<SwapChain::ImageViews> _p_imageViews;

    #ifndef NDEBUG
    std::optional<DebugMessenger> _debugMessenger;

    static constexpr bool _enableValidationLayers = true;
    #else
    static constexpr bool _enableValidationLayers = false;
    #endif

    static constexpr unsigned _windowWidth = 800;

    static constexpr unsigned _windowHeight = 600;
}; // struct Application::Impl


Application::Application()
    : _p_impl(new Impl)
{
    this->initWindow();
    this->initVulkan();
    this->initDebugMessenger();
    this->createSurface();
    this->createPhysicalDevice();
    this->createLogicalDevice();
    this->createSwapChain();
    this->createImageViews();
}



Application::~Application()
{
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
    _p_impl->_p_window = glfwCreateWindow(_p_impl->_windowWidth,
                                          _p_impl->_windowHeight,
                                          "vktutorial",
                                          nullptr,
                                          nullptr);
}

void Application::initVulkan()
{
    this->createVkInstance();
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
    for (uint32_t i=0; i<numberOfGLFWExtensions; ++i) {
        *it++ = *glfwExtensions++;
    }

    #if defined(__APPLE__) && __APPLE__
    *it++ = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    #endif

    if (_p_impl->_enableValidationLayers) {
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

    _p_impl->_debugMessenger.emplace(_p_impl->_p_vulkanInstance, createInfo);
    #endif
}

void Application::createVkInstance()
{
    std::vector<std::string> extensions;
    this->getRequiredExtensions(std::back_inserter(extensions));
    _p_impl->_p_vulkanInstance = std::make_shared<VulkanInstance>(extensions);
}


void Application::createSurface()
{
    _p_impl->_p_windowSurface = std::make_shared<WindowSurface>(_p_impl->_p_vulkanInstance, _p_impl->_p_window);
}


void Application::createPhysicalDevice()
{
    _p_impl->_p_physicalDevice = std::make_shared<PhysicalDevice>(VK_NULL_HANDLE);
    *_p_impl->_p_physicalDevice = PhysicalDevice::getDefaultDevice(
        _p_impl->_p_vulkanInstance->get(),
        _p_impl->_p_windowSurface->get()
    ).value();
}


void Application::createLogicalDevice()
{
    _p_impl->_p_logicalDevice = std::make_shared<GraphicsLogicalDevice>(_p_impl->_p_physicalDevice);
}


void Application::createSwapChain()
{
    _p_impl->_p_swapChain = std::make_shared<SwapChain>(_p_impl->_p_logicalDevice,
                                                        _p_impl->_p_windowSurface);
}


void Application::createImageViews()
{
    _p_impl->_p_imageViews = std::make_shared<SwapChain::ImageViews>(_p_impl->_p_swapChain);
}


void Application::mainLoop()
{
    while (!glfwWindowShouldClose(_p_impl->_p_window)) {
        glfwPollEvents();
        break;
    } // while not window_should_close
}
