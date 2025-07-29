cmake_minimum_required(VERSION 3.15)
project(VulkanDemo LANGUAGES CXX)
if(WIN32)
    set(CMAKE_CXX_COMPILER_TARGET "x86_64-w64-mingw32")
    
    # 使用 MinGW 的运行时库
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
    
    # 指定 MinGW 库路径（替换为你的路径）
    set(MINGW_PATH "E:/LLVM")
    include_directories(
        "${MINGW_PATH}/include"
        "${MINGW_PATH}/x86_64-w64-mingw32/include"
    )
    # link_directories(
    #     "${MINGW_PATH}/lib"
    #     "${MINGW_PATH}/x86_64-w64-mingw32/lib"
    # )
    
    # 使用 MinGW 的链接器
    set(CMAKE_LINKER "${MINGW_PATH}/bin/ld.exe")
    set(CMAKE_AR "${MINGW_PATH}/bin/ar.exe")
endif()

# 设置构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/..)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
link_directories("./lib")

# 包含项目头文件
include_directories(${CMAKE_SOURCE_DIR}/include)

# 收集源文件（推荐显式列出而非 GLOB）
file(GLOB SRC_FILES 
    "*.cpp"
    "Vkbase/*.cpp"
    "Modelbase/*.cpp"
)

# 创建可执行文件
add_executable(main ${SRC_FILES})

# 设置目标属性
target_compile_options(main PRIVATE
    -g
    -Wall -Wextra  # 启用警告
)

# 链接库
target_link_libraries(main
    glfw3
    assimp
    vulkan
)

# # 添加着色器编译自定义目标
# add_custom_target(compile_shaders
#     COMMAND ./compile.sh
#     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/shader
#     COMMENT "Compiling shaders..."
# )

# # 主目标依赖着色器编译
# add_dependencies(main compile_shaders)

# macOS特定设置
if(APPLE)
    # 设置运行路径
    set_target_properties(main PROPERTIES
        INSTALL_RPATH "@executable_path/lib"
        BUILD_WITH_INSTALL_RPATH ON
    )
    
    # 添加MoltenVK icd路径到运行路径
    set_target_properties(main PROPERTIES
        LINK_FLAGS "-Wl,-rpath,@executable_path/share/vulkan/icd.d/MoltenVK_icd.json"
    )
endif()