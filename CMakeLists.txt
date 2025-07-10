cmake_minimum_required(VERSION 3.15)
project(VulkanDemo LANGUAGES CXX)
set(CMAKE_BUILD_TYPE Debug)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/..)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 包含项目头文件
include_directories(${CMAKE_SOURCE_DIR}/include)

# 链接库目录
link_directories(${CMAKE_SOURCE_DIR}/lib)

# 收集源文件
file(GLOB SRC_FILES 
    "*.cpp"
    "Vkbase/*.cpp"
    "Modelbase/*.cpp"
)

# 创建可执行文件
add_executable(main ${SRC_FILES})

# 设置目标属性
target_compile_options(main PRIVATE
    -fcolor-diagnostics
    -fansi-escape-codes
    -g
)

# 链接库
target_link_libraries(main
    glfw.3
    assimp
    vulkan.1.3.296
)

# 添加着色器编译自定义目标
add_custom_target(compile_shaders
    COMMAND ./compile.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/shader
    COMMENT "Compiling shaders..."
)

# 主目标依赖着色器编译
add_dependencies(main compile_shaders)

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

# 安装目标（可选）
install(TARGETS main DESTINATION bin)