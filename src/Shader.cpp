// --- External Includes ---
#include "vulkan/vulkan.hpp"

// --- Internal Includes ---
#include "Shader.hpp"

// --- STL Includes ---
#include <fstream>
#include <vulkan/vulkan_core.h>


void ShaderIO::loadSpirv(std::istream& r_stream,
                         std::vector<char>& r_output) const
{
    constexpr std::size_t chunkSize = 0x400ul;
    std::size_t bytesRead = 0;

    while (!r_stream.eof()) {
        if (r_stream.fail()) {
            throw std::runtime_error("Error while reading shader");
        }

        // Resize the output buffer if there's not enough memory in it
        if (r_output.size() - bytesRead < chunkSize) {
            r_output.resize(bytesRead + chunkSize);
        }

        // Read the next chunk
        r_stream.read(r_output.data() + bytesRead,
                      r_output.size() - bytesRead);

        // Update counters
        bytesRead += r_stream.gcount();
    }

    // Trim the output
    r_output.resize(bytesRead);
}


SpirvShaderIO::SpirvShaderIO(std::filesystem::path&& r_spirv)
    : _spirv(std::move(r_spirv))
{
}


std::vector<char> SpirvShaderIO::load() const
{
    std::ifstream file(_spirv, std::ios::in | std::ios::binary | std::ios::ate);
    std::vector<char> output(static_cast<std::size_t>(file.tellg()));
    file.seekg(0);
    this->loadSpirv(file, output);
    file.close();
    return output;
}


struct Shader::Impl
{
    Impl(const ShaderIO& r_io,
         const LogicalDevice& r_device)
        : vulkanDevice(r_device.getDevice()),
          vulkanShader()
    {
        const auto spirv = r_io.load();

        VkShaderModuleCreateInfo info {};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = spirv.size();
        info.pCode = reinterpret_cast<const uint32_t*>(spirv.data());

        if (vkCreateShaderModule(this->vulkanDevice,
                                 &info,
                                 nullptr,
                                 &this->vulkanShader) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module\n");
    }
    }

    ~Impl()
    {
        vkDestroyShaderModule(this->vulkanDevice,
                              this->vulkanShader,
                              nullptr);
    }

    VkDevice vulkanDevice;

    VkShaderModule vulkanShader;
};


Shader::Shader(const ShaderIO& r_io,
               const LogicalDevice& r_device)
    : _p_impl(new Impl(r_io, r_device))
{
}


Shader::~Shader()
{
}
