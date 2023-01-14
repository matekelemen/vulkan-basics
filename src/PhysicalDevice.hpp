#pragma once

// --- External Includes ---
#include "vulkan/vulkan.hpp"

// --- Internal Includes ---
#include "utilities.hpp"
#include "VulkanInstance.hpp"

// --- STL Includes ---
#include <compare>
#include <memory>
#include <vector>
#include <array>
#include <optional>
#include <tuple>
#include <limits>


class PhysicalDevice
{
public:
    using UUID = std::array<uint8_t,VK_UUID_SIZE>;

    struct QueueFamily
    {
        std::optional<uint32_t> graphics;

        std::optional<uint32_t> presentation;
    }; // struct QueueFamily

public:
    PhysicalDevice(VkPhysicalDevice device)
        : _device(device)
    {
    }

    ///@name Member Access
    ///@{

    const VkPhysicalDevice& getDevice() const
    {
        return _device;
    }

    ///@}
    ///@name Queries
    ///@{

    VkPhysicalDeviceProperties getProperties() const
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(_device, &properties);
        return properties;
    }

    VkPhysicalDeviceFeatures getFeatures() const
    {
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(_device, &features);
        return features;
    }

    std::string getName() const
    {
        return this->getProperties().deviceName;
    }

    UUID getUUID() const
    {
        UUID id;
        const auto properties = this->getProperties();
        std::copy(properties.pipelineCacheUUID,
                  properties.pipelineCacheUUID + id.size(),
                  id.begin());
        return id;
    }

    QueueFamily getQueueFamily(std::optional<VkSurfaceKHR> surface) const
    {
        QueueFamily family;

        uint32_t numberOfFamilies = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(_device, &numberOfFamilies, nullptr);
        std::vector<VkQueueFamilyProperties> families(numberOfFamilies);
        vkGetPhysicalDeviceQueueFamilyProperties(_device, &numberOfFamilies, families.data());

        for (std::size_t i=0; i<families.size(); ++i) {
            const auto& r_family = families[i];
            assert(i < std::numeric_limits<uint32_t>::max());

            if (r_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                family.graphics.emplace(i);
            } // graphics

            if (surface.has_value()) {
                VkBool32 hasPresentationSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(_device,
                                                     i,
                                                     surface.value(),
                                                     &hasPresentationSupport);
                if (hasPresentationSupport)
                    family.presentation.emplace(i);
            } // presentation
        } // for family in families

        return family;
    }

    ///@}

    template <concepts::Iterator TOutputIt>
    static void getDevices(const VkInstance& r_vulkanInstance, TOutputIt it_out)
    {
        uint32_t numberOfDevices = 0;
        vkEnumeratePhysicalDevices(r_vulkanInstance, &numberOfDevices, nullptr);
        std::vector<VkPhysicalDevice> devices(numberOfDevices);
        vkEnumeratePhysicalDevices(r_vulkanInstance, &numberOfDevices, devices.data());
        for (const auto& r_device : devices) {
            *it_out++ = PhysicalDevice(r_device);
        }
    }

    static std::optional<PhysicalDevice> getDefaultDevice(const VkInstance& r_vulkanInstance,
                                                          const VkSurfaceKHR& r_surface)
    {
        std::vector<PhysicalDevice> devices;
        PhysicalDevice::getDevices(r_vulkanInstance, std::back_inserter(devices));
        std::vector<std::tuple<
            PhysicalDevice,
            VkPhysicalDeviceProperties,
            VkPhysicalDeviceFeatures,
            QueueFamily
        >> deviceParams;
        deviceParams.reserve(devices.size());
        std::transform(devices.begin(),
                       devices.end(),
                       std::back_inserter(deviceParams),
                       [&r_surface](PhysicalDevice device) {
                           return std::make_tuple(
                               device,
                               device.getProperties(),
                               device.getFeatures(),
                               device.getQueueFamily(r_surface)
                           ); // make_tuple
                       }); // std::transform

        std::erase_if(
            deviceParams,
            [](const auto& r_tuple) -> bool
            {
                const auto& r_properties = std::get<1>(r_tuple);
                const auto& r_features = std::get<2>(r_tuple);
                const auto& r_family = std::get<3>(r_tuple);

                bool isSuitable = true;
                isSuitable &= (r_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
                               || r_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
                isSuitable &= r_features.geometryShader;
                isSuitable &= r_family.presentation.has_value();

                return !isSuitable;
            } // erasePredicate
        ); // std::erase_if

        std::optional<PhysicalDevice> pick;
        if (!devices.empty()) {
            // Order the devices
            std::sort(deviceParams.begin(),
                      deviceParams.end(),
                      [](const auto& r_left, const auto& r_right) -> bool {
                          const auto& r_lProp = std::get<1>(r_left);
                          const auto& r_rProp = std::get<1>(r_right);
                          const auto& r_lFeats = std::get<2>(r_left);
                          const auto& r_rFeats = std::get<2>(r_right);
                          const auto& r_lFamily = std::get<3>(r_left);
                          const auto& r_rFamily = std::get<3>(r_right);

                          // Prefer discrete GPUs
                          if (r_lProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && r_rProp.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                              return true;

                          // Prefer GPUs with more memory
                          if (r_lProp.limits.maxImageDimension2D > r_rProp.limits.maxImageDimension2D)
                              return true;

                          // Prefer GPUs that support graphics and presentation on the same queue
                          if ((r_lFamily.graphics.value() == r_lFamily.presentation.value()) && (r_rFamily.graphics.value() != r_rFamily.presentation.value()))
                            return true;

                          // Prefer GPUs with 64 bit floating point support
                          if (r_lFeats.shaderFloat64 && !r_rFeats.shaderFloat64)
                            return true;

                          return false;
                      }); // comparisonFunction
            pick.emplace(devices.front());
        } // if devices

        return pick;
    }

private:
    VkPhysicalDevice _device;
}; // class PhysicalDevice
