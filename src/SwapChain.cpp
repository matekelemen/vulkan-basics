// --- External Includes ---
#include "LogicalDevice.hpp"
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// --- Internal Includes ---
#include "SwapChain.hpp"

// --- STL Includes ---
#include <limits>
#include <algorithm>
#include <array>


SwapChain::Properties SwapChain::Properties::query(const PhysicalDevice& r_device,
                                                   const WindowSurface& r_surface)
{
    Properties properties;

    // Query queue family
    properties._queueFamily = r_device.getQueueFamily(r_surface.get());

    // Query physical device extensions
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(r_device.getDevice(),
                                         nullptr,
                                         &extensionCount,
                                         nullptr);
    if (0 < extensionCount) {
        properties._extensions.resize(extensionCount);
        vkEnumerateDeviceExtensionProperties(r_device.getDevice(),
                                             nullptr,
                                             &extensionCount,
                                             properties._extensions.data());
    }

    // Query capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r_device.getDevice(),
                                              r_surface.get(),
                                              &properties._capabilities);

    // Query surface formats
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(r_device.getDevice(),
                                         r_surface.get(),
                                         &formatCount,
                                         nullptr);
    if (0 < formatCount) {
        properties._formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(r_device.getDevice(),
                                             r_surface.get(),
                                             &formatCount,
                                             properties._formats.data());
    }

    // Query present modes
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(r_device.getDevice(),
                                              r_surface.get(),
                                              &presentModeCount,
                                              nullptr);
    if (0 < presentModeCount) {
        properties._presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(r_device.getDevice(),
                                                  r_surface.get(),
                                                  &presentModeCount,
                                                  properties._presentModes.data());
    }

    return properties;
}


const PhysicalDevice::QueueFamily& SwapChain::Properties::getQueueFamily() const noexcept
{
    return _queueFamily;
}


const std::vector<VkExtensionProperties> SwapChain::Properties::getDeviceExtensions() const noexcept
{
    return _extensions;
}


const VkSurfaceCapabilitiesKHR& SwapChain::Properties::getCapabilities() const noexcept
{
    return _capabilities;
}


const std::vector<VkSurfaceFormatKHR>& SwapChain::Properties::getSurfaceFormats() const noexcept
{
    return _formats;
}


const std::vector<VkPresentModeKHR>& SwapChain::Properties::getPresentModes() const noexcept
{
    return _presentModes;
}


SwapChain::ImageViews::View::View(SwapChain& r_swapChain,
                                  std::size_t i_image)
    : _view(),
      _image(),
      _device(r_swapChain.getLogicalDevice().getDevice())
{
    if (r_swapChain.getImages().size() <= i_image) {
        throw std::runtime_error("Image view index out of range for swap chain of size " + std::to_string(r_swapChain.getImages().size()));
    }
    _image = r_swapChain.getImages()[i_image];

    if (r_swapChain.getProperties()._formats.empty()) {
        throw std::runtime_error("The swap chain's image format does not exist\n");
    }

    VkImageViewCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.image = _image;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = r_swapChain.getProperties()._formats.front().format;
    info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(r_swapChain.getLogicalDevice().getDevice(),
                          &info,
                          nullptr,
                          &_view) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image view");
    }
}


VkImageView SwapChain::ImageViews::View::get()
{
    return _view;
}


SwapChain::ImageViews::View::~View()
{
    vkDestroyImageView(_device, _view, nullptr);
}


namespace {


/// @brief Choose the most suitable surface format for @ref SwapChain.
VkSurfaceFormatKHR chooseSurfaceFormat(const SwapChain::Properties& r_properties)
{
    if (r_properties.getSurfaceFormats().empty()) {
        throw std::runtime_error("No surface formats available in the provided swap chain properties");
    }

    // Each VkSurfaceFormatKHR has:
    // - "format" that defined color channels and types
    // - "colorSpace" that describes whether the format supports SRGB

    // Prefer BGR/SRGB
    for (const auto& r_format : r_properties.getSurfaceFormats()) {
        if (r_format.format == VK_FORMAT_B8G8R8A8_SRGB && r_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return r_format;
        }
    }

    return r_properties.getSurfaceFormats().front();
}


/// @brief Choose the most suitable presentation mode for @ref SwapChain.
/// @details Available modes in vulkan:
///          - VK_PRESENT_MODE_IMMEDIATE_KHR: (display images as soon as they are submitted)
///          - VK_PRESENT_MODE_FIFO_KHR: push submitted images to a queue, from which vulkan grabs the oldest
///          - VK_PRESENT_MODE_FIFO_RELAXED_KHR: if submission is late, the presentation temporarily switches to immediate mode
///          - VK_PRESENT_MODE_MAILBOX_KHR: if submission is faster than the display speed, existing frames are overwritten in the queue
VkPresentModeKHR choosePresentMode(const SwapChain::Properties& r_properties,
                                   VkPresentModeKHR preferred = VK_PRESENT_MODE_FIFO_KHR)
{
    if (r_properties.getPresentModes().empty()) {
        throw std::runtime_error("No presentation modes available in the provided swap chain properties");
    }

    // Return the preferred mode if available
    const auto& r_modes = r_properties.getPresentModes();
    for (const auto& r_mode : r_modes) {
        if (r_mode == preferred) {
            return r_mode;
        }
    }

    // VK_PRESENT_MODE_FIFO_KHR is supposed to be guaranteed, but you never know so I'll check it
    if (std::find(r_modes.begin(),
                  r_modes.end(),
                  VK_PRESENT_MODE_FIFO_KHR) == r_modes.end()) {
        throw std::runtime_error("Could not find FIFO present mode in swap chain properties");
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}


VkExtent2D chooseSwapExtent(const SwapChain::Properties& r_properties,
                            const WindowSurface& r_surface)
{
    const auto& r_capabilities = r_properties.getCapabilities();
    if (r_capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
        // In this case, the swap's resolution can be set to a
        // value different than that of the window.
        int width = 0, height = 0;
        glfwGetFramebufferSize(r_surface.getWindow(), &width, &height);

        VkExtent2D extent {static_cast<uint32_t>(width),
                           static_cast<uint32_t>(height)};
        std::clamp(extent.width,
                   r_capabilities.minImageExtent.width,
                   r_capabilities.maxImageExtent.width);
        std::clamp(extent.height,
                   r_capabilities.minImageExtent.height,
                   r_capabilities.maxImageExtent.height);
        return extent;
    } else {
        // The window's and swap's resolution must be identical.
        return r_capabilities.currentExtent;
    }
}


uint32_t chooseSwapChainSize(const SwapChain::Properties& r_properties)
{
    const auto& r_capabilities = r_properties.getCapabilities();
    uint32_t output = r_capabilities.minImageCount + 1;

    if (r_capabilities.maxImageCount != 0) {
        // Swap chain size is limited
        std::clamp(output,
                   r_capabilities.minImageCount,
                   r_capabilities.maxImageCount);
    }

    return output;
}


} // unnamed namespace


SwapChain::SwapChain(const std::shared_ptr<GraphicsLogicalDevice>& rp_device,
                     const std::shared_ptr<WindowSurface>& rp_surface)
    : _p_device(rp_device),
      _p_surface(rp_surface),
      _swapChain(),
      _images()
{
    const Properties properties = this->getAvailableProperties();

    // Check whether all requirements are met
    if (!SwapChain::checkRequirements(properties)) {
        std::stringstream message;
        message << "Physical device '"
                << _p_device->getPhysicalDevice().getName()
                << "' does not meet SwapChain requirements\n";
        throw std::runtime_error(message.str());
    }

    // Choose swap chain properties based on what's available
    const auto surfaceFormat = chooseSurfaceFormat(properties);
    const auto presentMode = choosePresentMode(properties);
    const auto swapExtent = chooseSwapExtent(properties, *_p_surface);
    const auto swapChainSize = chooseSwapChainSize(properties);

    // Assemble the swap chain constructor info
    VkSwapchainCreateInfoKHR info {};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = _p_surface->get();
    info.imageFormat = surfaceFormat.format;
    info.imageColorSpace = surfaceFormat.colorSpace;
    info.imageExtent = swapExtent;
    info.minImageCount = swapChainSize;
    info.presentMode = presentMode;

    info.imageArrayLayers = 1; // <== may be greater than 1 if rendering to a 3D image

    // Specify the type of operations the images in the
    // swap chain will be used for.
    // - VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT: render directly to the image
    // - VK_IMAGE_USAGE_TRANSFER_DST_BIT: @todo ?
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Decide how the graphics and presentation queues should communicate their images.
    // - if the two queues are actually the same, there are no ownership issues
    // - ask vulkan to manage ownership transfers automatically if the two queues are distinct
    const auto queueFamily = _p_device->getPhysicalDevice().getQueueFamily(_p_surface->get());
    std::array<uint32_t,2> queueFamilyIDs {queueFamily.graphics.value(),
                                           queueFamily.presentation.value()};

    if (queueFamily.graphics.value() == queueFamily.presentation.value()) {
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.queueFamilyIndexCount = 0; // <== @todo
        info.pQueueFamilyIndices = nullptr; // <== @todo
    } else {
        info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = 2;
        info.pQueueFamilyIndices = queueFamilyIDs.data();
    }

    // Set transformations performed on every submitted image.
    // (rotation, scaling, etc.)
    // By default, no transform is carried out.
    info.preTransform = properties.getCapabilities().currentTransform;

    // Choose whether to blend the rendered images with the window's background
    // using the alpha channels in the submitted images.
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // Ignore invisible pixels (eg.: due to overlapping windows)
    info.clipped = VK_TRUE;

    // Decide what happens to invalidated images in the swap chain
    // (eg.: due to window resizing).
    info.oldSwapchain = VK_NULL_HANDLE;

    // Finally ... construct the bloody swap chain
    if (vkCreateSwapchainKHR(_p_device->getDevice(),
                             &info,
                             nullptr,
                             &_swapChain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to construct swap chain\n");
    }

    // Get the images in the constructed swap chain
    uint32_t finalSwapChainSize = 0;
    vkGetSwapchainImagesKHR(_p_device->getDevice(),
                            _swapChain,
                            &finalSwapChainSize,
                            nullptr);
    _images.resize(finalSwapChainSize);
    vkGetSwapchainImagesKHR(_p_device->getDevice(),
                            _swapChain,
                            &finalSwapChainSize,
                            _images.data());

    // Populate the properties the swap chain ended up with
    _properties._queueFamily = properties.getQueueFamily();
    _properties._extensions = properties.getDeviceExtensions();
    _properties._capabilities = properties.getCapabilities();
    _properties._formats = {surfaceFormat};
    _properties._presentModes = {presentMode};
}


SwapChain::~SwapChain()
{
    vkDestroySwapchainKHR(_p_device->getDevice(),
                          _swapChain,
                          nullptr);
}


SwapChain::Properties SwapChain::getAvailableProperties() const
{
    return Properties::query(_p_device->getPhysicalDevice(), *_p_surface);
}


const SwapChain::Properties& SwapChain::getProperties() const noexcept
{
    return _properties;
}


std::vector<VkImage>& SwapChain::getImages() noexcept
{
    return _images;
}


const GraphicsLogicalDevice& SwapChain::getLogicalDevice() const noexcept
{
    return *_p_device;
}


GraphicsLogicalDevice& SwapChain::getLogicalDevice() noexcept
{
    return *_p_device;
}


bool SwapChain::checkRequirements(const Properties& r_properties) noexcept
{
    return    SwapChain::checkQueueRequirements(r_properties)
           && SwapChain::checkExtensionRequirements(r_properties)
           && SwapChain::checkSurfaceFormatRequirements(r_properties)
           && SwapChain::checkPresentModeRequirements(r_properties)
           ;
}


bool SwapChain::checkQueueRequirements(const Properties& r_properties) noexcept
{
    return r_properties.getQueueFamily().all();
}


bool SwapChain::checkExtensionRequirements(const Properties& r_properties) noexcept
{
    const auto& r_availableExtensions = r_properties.getDeviceExtensions();

    // Check whether all required extensions' names are in the collected (available) extensions
    std::vector<const char*> requiredExtensions;
    SwapChain::getRequiredExtensions(std::back_inserter(requiredExtensions));
    return std::all_of(requiredExtensions.begin(),
                       requiredExtensions.end(),
                       [&r_availableExtensions](const char* p_required) -> bool {
                           return std::find_if(r_availableExtensions.begin(),
                                               r_availableExtensions.end(),
                                               [p_required](const auto& r_available) -> bool {
                                                   return std::strcmp(p_required, r_available.extensionName);
                                               }) != r_availableExtensions.end();
                       });
}


bool SwapChain::checkSurfaceFormatRequirements(const Properties& r_properties) noexcept
{
    return !r_properties.getSurfaceFormats().empty();
}


bool SwapChain::checkPresentModeRequirements(const Properties &r_properties) noexcept
{
    return !r_properties.getPresentModes().empty();
}

