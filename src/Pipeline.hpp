#pragma once

// --- External Includes ---
#include "vulkan/vulkan.hpp"

// --- Internal Includes ---
#include "Shader.hpp"
#include "LogicalDevice.hpp"

// --- STL Includes ---
#include <optional>


class Pipeline
{
public:
    Pipeline(std::optional<std::shared_ptr<ShaderIO>> p_vertexShaderIO,
             std::optional<std::shared_ptr<ShaderIO>> p_fragmentShaderIO)
    {
    }

};
