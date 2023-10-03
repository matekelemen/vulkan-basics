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

    class ImageViews
    {
    public:
        class View
        {
        public:
            View() = delete;

            View(View&&) noexcept = default;

            View(const View&) = delete;

            View(SwapChain& r_swapChain, std::size_t i_image);

            VkImageView get();

            virtual ~View();

        private:
            VkImageView _view;

            VkImage _image;

            VkDevice _device;
        }; // class View

    public:
        ImageViews(const std::shared_ptr<SwapChain> rp_swapChain)
            : ImageViews(rp_swapChain,
                         [](SwapChain& r_chain, std::size_t i_image) -> std::unique_ptr<View> {
                            return std::make_unique<View>(r_chain, i_image);
                         })
        {}

        /// @tparam TViewFactory Factory function producing a unique pointer to a @ref View.
        /// @tparam TFactoryArgs Additional arguments passed to @a TViewFactory after @ref SwapChain and the image index.
        template <class TViewFactory, class ...TFactoryArgs>
        ImageViews(const std::shared_ptr<SwapChain> rp_swapChain,
                   const TViewFactory& r_factory,
                   const TFactoryArgs& ... r_factoryArgs)
            : _views(),
              _p_swapChain(rp_swapChain)
        {
            const std::size_t imageCount = rp_swapChain->getImages().size();
            _views.reserve(imageCount);
            for (std::size_t i_image=0; i_image<imageCount; ++i_image) {
                _views.emplace_back(r_factory(*rp_swapChain,
                                              i_image,
                                              r_factoryArgs...));
            }
        }

    private:
        std::vector<std::unique_ptr<View>> _views;

        std::shared_ptr<SwapChain> _p_swapChain;
    }; // class ImageViews

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
    const Properties& getProperties() const noexcept;

    /// @brief Access images in the swap chain.
    std::vector<VkImage>& getImages() noexcept;

    const GraphicsLogicalDevice& getLogicalDevice() const noexcept;

    GraphicsLogicalDevice& getLogicalDevice() noexcept;

    /// @}
    /// @name Checks
    /// @{

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
