cmake_minimum_required(VERSION 3.10)
project(ffmpeg_demo)

# 设置ffmpeg依赖库及头文件所在目录，并存进指定变量
set(ffmpeg_libs_DIR /lib/aarch64-linux-gnu)
set(ffmpeg_headers_DIR /usr/local/include)



# 添加头文件路径到编译器的头文件搜索路径下，多个路径以空格分隔
include_directories( ${ffmpeg_headers_DIR} )
link_directories(${ffmpeg_libs_DIR} )


set(CMAKE_CXX_STANDARD 14)
add_executable(ffmpeg_demo main.cpp)
target_link_libraries(${PROJECT_NAME} avcodec avformat avutil swresample swscale swscale avfilter )
