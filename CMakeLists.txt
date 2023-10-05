cmake_minimum_required(VERSION 3.12)
project(vktutorial CXX)

set(CMAKE_CXX_STANDARD 20)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_BINARY_DIR}/compile_commands.json" "${CMAKE_SOURCE_DIR}/compile_commands.json")

file(GLOB sources "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(${PROJECT_NAME} ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME} glfw Vulkan::Vulkan)

# @todo get validation layers working on MacOS
if(APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE "-DNDEBUG")
endif()

# Shaders
if(APPLE)
    find_program(glslc glslang REQUIRED)
else()
    find_program(glslc NAMES glslc HINTS Vulkan::glslc REQUIRED)
endif()
file(GLOB shader_sources "${CMAKE_CURRENT_SOURCE_DIR}/src/shader/*.vert"
                         "${CMAKE_CURRENT_SOURCE_DIR}/src/shader/*.frag")

set(spirvs "")
foreach(shader_source ${shader_sources})
    get_filename_component(shader_name "${shader_source}" NAME)
    set(spirv "${CMAKE_BINARY_DIR}/shaders/${shader_name}.spv")
    if(APPLE)
        add_custom_command(OUTPUT "${spirv}"
                           COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_BINARY_DIR}/shaders"
                           COMMAND "${glslc}"
                                   -G
                                   --target-env vulkan1.2
                                   -o "${spirv}"
                                   "${shader_source}")
    else()
        add_custom_command(OUTPUT "${spirv}"
                           COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_BINARY_DIR}/shaders"
                           COMMAND "${glslc}"
                                   -MD -MF "${shader_name}.d"
                                   --target-env=vulkan
                                   -o "${spirv}"
                                   "${shader_source}"
                           DEPFILE "${shader_name}.d")
    endif()
    list(APPEND spirvs "${spirv}")
endforeach()

add_custom_target(shaders DEPENDS ${spirvs})
add_dependencies(${PROJECT_NAME} shaders)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
                   COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_BINARY_DIR}/shaders" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders")
