
cmake_minimum_required(VERSION 2.8)
project( DisplayImage )
SET("OpenCV_DIR" "/work/opencv-3.2.0")
find_package( OpenCV REQUIRED )
file(GLOB SOURCES
*.h
*.cpp)
add_executable( DisplayImage ${SOURCES} )
target_link_libraries( DisplayImage ${OpenCV_LIBS} )

