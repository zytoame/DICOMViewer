cmake_minimum_required(VERSION 3.5)
project(DICOMViewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Qt路径
set(CMAKE_PREFIX_PATH "D:/Qt5.15.2/5.15.2/msvc2019_64")
# VTK路径
set(VTK_DIR "D:/VTK/install/lib/cmake/vtk-9.1")

# Qt配置
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 构建类型
set(CMAKE_BUILD_TYPE Release)
set(BUILD_SHARED_LIBS ON)

# VTK配置
set(VTK_FORBID_DOWNLOADS TRUE)
set(VTK_GROUP_ENABLE_RENDERING ON)
set(VTK_MODULE_ENABLE_VTK_RenderingOpenGL2 YES)
set(VTK_MODULE_ENABLE_VTK_RenderingQt YES)

# VTK配置
find_package(VTK REQUIRED)

# Qt配置
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Widgets Gui REQUIRED) # 添加 Gui 模块

# 源文件
aux_source_directory(./src srcs)

# MSVC UTF-8编码
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# 可执行文件
add_executable(${PROJECT_NAME} WIN32 ${srcs})

# 链接库
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    Qt5::Widgets
    ${VTK_LIBRARIES}
)

# 现代CMake方式设置VTK目标
if (VTK_VERSION VERSION_GREATER_EQUAL "8.90.0")
    vtk_module_autoinit(
        TARGETS ${PROJECT_NAME}
        MODULES ${VTK_LIBRARIES}
    )
endif()