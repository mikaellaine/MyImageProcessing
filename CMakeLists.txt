
cmake_minimum_required(VERSION 2.8)
project( ImageClustering )
SET("OpenCV_DIR" "/work/opencv-3.2.0")
find_package( OpenCV REQUIRED )
file(GLOB SOURCES
*.h
*.cpp)
add_executable( ImageClustering ${SOURCES} )
target_link_libraries( ImageClustering ${OpenCV_LIBS} )

