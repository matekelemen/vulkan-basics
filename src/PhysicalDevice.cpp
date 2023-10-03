// --- Internal Includes ---
#include "PhysicalDevice.hpp"

// --- STL Includes ---
#include <iostream>


std::ostream& operator<<(std::ostream& r_stream, const PhysicalDevice& r_device)
{
    return r_stream << r_device.getName();
}
