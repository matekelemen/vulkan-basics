#pragma once

// --- External Includes ---
#include "vulkan/vulkan.hpp"

// --- Internal Includes ---
#include "utilities.hpp"

// --- STL Includes ---
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>


class VulkanInstance
{
public:
    template <concepts::Container<std::string> TContainer>
    VulkanInstance(const TContainer& requiredExtensions)
    {
        // Convert extension names to C strings
        std::vector<const char*> cStrings(requiredExtensions.size());
        std::transform(requiredExtensions.begin(),
                       requiredExtensions.end(),
                       cStrings.begin(),
                       [](const auto& r_name) {return r_name.c_str();});

        // Optional info struct for optimizing vulkan calls
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
            debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                                            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = VulkanInstance::debugCallback;
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.pNext = nullptr;
        }

        // Create vulkan instance
        if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create vulkan instance");
        }
    }

    ~VulkanInstance()
    {
        vkDestroyInstance(_instance, nullptr);
    }

    ///@name Member Access
    ///@{

    VkInstance get() noexcept
    {
        return _instance;
    }

    ///@}

private:
    template <concepts::Iterator<std::string> TInputIt, concepts::Iterator TItOutput>
    static void getSupportedLayers(TInputIt begin, TInputIt end, TItOutput it_out)
    {
        uint32_t numberOfLayers;
        vkEnumerateInstanceLayerProperties(&numberOfLayers, nullptr);
        std::vector<VkLayerProperties> availableLayers(numberOfLayers);
        vkEnumerateInstanceLayerProperties(&numberOfLayers, availableLayers.data());

        for (; begin!=end; ++begin) {
            if (std::any_of(availableLayers.begin(),
                            availableLayers.end(),
                            [begin](const auto& r_name)
                                {return r_name.layerName == *begin;})) {
                *it_out++ = *begin;
            } // if validationLayer in availableLayers
        } // for layerName in validationLayers
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                        VkDebugUtilsMessageTypeFlagsEXT type,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* p_data,
                                                        void* p_userData)
    {
        std::cerr << "VulkanInstance";

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

private:
    VkInstance _instance;

    #ifndef NDEBUG
    static constexpr bool _enableValidationLayers = true;
    #else
    static constexpr bool _enableValidationLayers = false;
    #endif
}; // class VulkanInstance
