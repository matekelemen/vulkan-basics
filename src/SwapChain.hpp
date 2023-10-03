#pragma once

// --- Internal Includes ---
#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"
#include "WindowSurface.hpp"

// --- STL Includes ---
#include <cstdint>
#include <memory>
#include <cstring>
#include <sstream>
#include <vector>


class SwapChain
{
public:
    class Properties
    {
    public:
        static Properties query(const PhysicalDevice& r_physicalDevice,
                                const WindowSurface& r_windowSurface);

        /// @name Member Access
        /// @{

        const PhysicalDevice::QueueFamily& getQueueFamily() const noexcept;

        const std::vector<VkExtensionProperties> getDeviceExtensions() const noexcept;

        const VkSurfaceCapabilitiesKHR& getCapabilities() const noexcept;

        const std::vector<VkSurfaceFormatKHR>& getSurfaceFormats() const noexcept;

        const std::vector<VkPresentModeKHR>& getPresentModes() const noexcept;

        /// @}

    private:
        PhysicalDevice::QueueFamily _queueFamily;

        std::vector<VkExtensionProperties> _extensions;

        VkSurfaceCapabilitiesKHR _capabilities;

        std::vector<VkSurfaceFormatKHR> _formats;

        std::vector<VkPresentModeKHR> _presentModes;

    private:
        friend class SwapChain;
    }; // class Properties

public:
    SwapChain(const std::shared_ptr<GraphicsLogicalDevice>& rp_device,
              const std::shared_ptr<WindowSurface>& rp_surface);

    ~SwapChain();

    /// @name Queries
    /// @{

    /// @brief Get all required physical device extensions to support a @ref SwapChain.
    /// @tparam TIterator output iterator with @a const @a char* as value type.
    /// @return the output iterator pointing to the new end of the modified container.
    template <class TIterator>
    static TIterator getRequiredExtensions(TIterator it_output)
    {
        return GraphicsLogicalDevice::getRequiredExtensions(it_output);
    }

    /// @brief Get the swap chain related properties of the @ref PhysicalDevice and @ref SurfaceFormat.
    Properties getAvailableProperties() const;

    /// @brief Get the @ref Properties the @ref SwapChain was constructed with.
    Properties getProperties() const;

    /// @brief Check whether the provided properties are suitable for swap chains.
    static bool checkRequirements(const Properties& r_properties) noexcept;

    /// @brief Check whether the provided @ref SwapChain::Properties has both graphics and presentation support in its device queue.
    static bool checkQueueRequirements(const Properties& r_properties) noexcept;

    /// @brief Check whether all required device extensions available in the provided @ref SwapChain::Properties.
    static bool checkExtensionRequirements(const Properties& r_properties) noexcept;

    /// @brief Check whether the provided @ref SwapChain::Properties has at least one surface format required by @ref SwapChain.
    static bool checkSurfaceFormatRequirements(const Properties& r_properties) noexcept;

    /// @brief Check whether the provided @ref SwapChain::Properties has at least one present mode.
    static bool checkPresentModeRequirements(const Properties& r_properties) noexcept;

    /// @}

private:
    std::shared_ptr<GraphicsLogicalDevice> _p_device;

    std::shared_ptr<WindowSurface> _p_surface;

    VkSwapchainKHR _swapChain;

    std::vector<VkImage> _images;

    Properties _properties;
}; // class SwapChain
