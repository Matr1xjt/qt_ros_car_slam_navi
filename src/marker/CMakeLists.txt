#指定项目所需的最低 CMake 版本。
cmake_minimum_required(VERSION 3.0.2)
#定义项目的名称
project(marker)

## Compile as C++11, supported in ROS Kinetic and newer
# add_compile_options(-std=c++11)

#查找并导入外部依赖库。
find_package(catkin REQUIRED COMPONENTS
  roscpp
  sensor_msgs
  image_transport
  cv_bridge
  nav_msgs
)
find_package(Qt5  REQUIRED COMPONENTS  Widgets  )
#设置自动生成moc文件，一定要设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)#设置工程包含当前目录，非必须
#这用于指定包的依赖关系，由依赖于这个包的其他软件包使用。
catkin_package(
#包的导出包含路径
  INCLUDE_DIRS include
#从项目中导出的库
#  LIBRARIES marker
#本项目依赖的其他catkin项目
#  CATKIN_DEPENDS roscpp
#此项目所依赖的非 catkin CMake 项目
  DEPENDS system_lib
)

#添加头文件搜索路径，编译器使用这些目录来查找头文件,第一个参数“include”表示包中的include/目录也是路径的一部分。
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)

#指定将要构建的可执行文件的名称，以及它所依赖的源文件
add_executable(${PROJECT_NAME}_node
                                    src/marker_test.cpp
                                    src/basic.cpp
                                    src/basic.ui
                                    )
#设置可执行文件依赖的库。
target_link_libraries(${PROJECT_NAME}_node ${catkin_LIBRARIES})
target_link_libraries(${PROJECT_NAME}_node  Qt5::Widgets )#链接Qt库
