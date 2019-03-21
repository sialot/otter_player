# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("otter_player")
 
include_directories("include")
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 指定源文件
aux_source_directory(. DIR_SRCS) 

# 指定生成目标
add_executable(test ${DIR_SRCS}  ${SYS_SRCS})

target_link_libraries(test "avcodec.lib" "avformat.lib" "avutil.lib" "swscale.lib")

# TODO: 如有需要，请添加测试并安装目标。
 