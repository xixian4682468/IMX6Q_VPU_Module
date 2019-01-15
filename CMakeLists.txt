# 版本限定 
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)     
# 项目名称 
PROJECT(qsImgLib)
# 设置编译器选项：O3优化，显示所有警告
SET(CMAKE_C_FLAGS "$ENV{CFLAGS} -O3 -Wall -g")
set(CMAKE_C_COMPILER "arm-none-linux-gnueabi-gcc")
set(CMAKE_CXX_COMPILE "arm-none-linux-gnueabi-g++")

# 定义头文件的路径变量
SET(INCLUDE_PATH ${PROJECT_SOURCE_DIR}/include) 

#MESSAGE(STATUS "Include Path, ${INCLUDE_PATH}")

#头文件
#include_directories(${PROJECT_SOURCE_DIR})
#include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(/home/xixian/my-imx6/ffmpeg/ffmpeglib/include)

# 定义库文件路径
SET(LIB_PATH ${PROJECT_SOURCE_DIR}/lib)
MESSAGE(STATUS "Libs Path, ${LIB_PATH}")

# 定义源文件路径变量
SET(SOURCE_PATH ${PROJECT_SOURCE_DIR}/src) 
MESSAGE(STATUS "Source Path , ${SOURCE_PATH}") 

# 包含头文件搜索路径
INCLUDE_DIRECTORIES(${INCLUDE_PATH}) 

# 包含库搜索路径
LINK_DIRECTORIES(${LIB_PATH} ./lib)
LINK_DIRECTORIES(${LIB_PATH} /home/xixian/my-imx6/ffmpeg/ffmpeglib/lib)

# 定义待链接库名字
# 链接静态库
#SET(LIBS_LIST libmymath.a)   
                                                                                  
# 链接动态库
#SET(LIBS_LIST libavcodec.so libavdevice.so libavfilter.so libavformat.so libavutil.so libpostproc.so libswresample.so libswscale.so)
SET(LIBS_LIST libqsImgLib.so libipu.so libvpu.so libavcodec.so libavformat.so)
# 添加源文件路径下所有源文件存放到变量中(*.c && *.cpp)
AUX_SOURCE_DIRECTORY(${SOURCE_PATH} SRC_LIST)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

#set(OpenCV_DIR /home/xiangang/xiangang/opencv/arm-opencv-3.3.0)  
#set(OpenCV_LIBS /home/xiangang/xiangang/opencv/arm-opencv-3.3.0/lib)  
#include_directories(/home/xiangang/xiangang/opencv/arm-opencv-3.3.0/include) 

#find_package( OpenCV REQUIRED )

# 设置生成可执行文件的名称
SET(EXECUTABLE_FILE_NAME Camshift&NCC_92x92_Alashan.out)

# 生成可执行文件
ADD_EXECUTABLE(${EXECUTABLE_FILE_NAME} ${SRC_LIST})

# 设置可执行文件链接的库名称
TARGET_LINK_LIBRARIES(${EXECUTABLE_FILE_NAME} ${LIBS_LIST} -lm -lpthread -lrt -ldl)
