# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.5)

# 项目信息
project (testGPS)
set(CMAKE_CXX_STANDARD 14)


#GeographicLib库
find_package (GeographicLib 1.49 REQUIRED)

# 添加 include 子目录
set(HEADER_DIRECTORY ${HEADER_DIRECTORY} 
    "/home/scar1et/serial/serial/include"
        "/home/scar1et/sqlite_modern_cpp-master/hdr"
    "/home/scar1et/Desktop/GPSv2")

set(LIBRARY_DIRECTORY ${LIBRARY_DIRECTORY} 
    "/usr/lib"
     "/usr/lib//x86_64-linux-gnu"
    "/tmp/usr/local/lib/")
    
# 指定生成目标
add_executable (testGPS testGPS.cpp)
target_include_directories(testGPS PRIVATE ${HEADER_DIRECTORY})
find_library(SQLITE3_LIBRARY sqlite3  ${LIBRARY_DIRECTORY})
target_link_libraries(testGPS  ${SQLITE3_LIBRARY})
find_library(SERIAL_LIBRARY serial ${LIBRARY_DIRECTORY})
target_link_libraries(testGPS ${SERIAL_LIBRARY})
target_link_libraries(testGPS ${GeographicLib_LIBRARIES})