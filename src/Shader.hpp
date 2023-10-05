#pragma once

// --- Internal Includes ---
#include "LogicalDevice.hpp"

// --- STL Includes ---
#include <filesystem>
#include <iosfwd>
#include <vector>


class ShaderIO
{
public:
    virtual ~ShaderIO() = default;

    virtual std::vector<char> load() const = 0;

protected:
    void loadSpirv(std::istream& r_stream,
                   std::vector<char>& r_output) const;
}; // class ShaderIO



class SpirvShaderIO final : public ShaderIO
{
public:
    SpirvShaderIO(std::filesystem::path&& r_spirv);

    std::vector<char> load() const override;

private:
    std::filesystem::path _spirv;
}; // class SpirvShaderIO


class Shader
{
public:
    Shader(const ShaderIO& r_io,
           const LogicalDevice& r_device);

    ~Shader();

private:
    struct Impl;
    std::unique_ptr<Impl> _p_impl;
}; // class Shader
