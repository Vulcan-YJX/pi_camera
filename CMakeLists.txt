cmake_minimum_required(VERSION 3.10)
project(CSI_Camera_Example)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0)
find_package(OpenCV REQUIRED)
if (OpenCV_FOUND)
	message("Found OpenCV")
	message("Includes:" ${OpenCV_INCLUDE_DIRS})
endif(OpenCV_FOUND)

include_directories(
    include
    ${OpenCV_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
)

link_directories(${GSTREAMER_LIBRARY_DIRS})

add_executable(csi_camera gst_cam.cpp)

add_executable(csi_camera_example test.cpp picamera.cpp)
target_include_directories(csi_camera_example PRIVATE ${LIBCAMERA_INCLUDE_DIRS})
target_link_libraries(csi_camera_example PRIVATE ${LIBCAMERA_LIBRARIES} ${OpenCV_LIBS})

target_link_libraries(csi_camera
    ${GSTREAMER_LIBRARIES}
    ${OpenCV_LIBS}
)
add_definitions(${GSTREAMER_CFLAGS_OTHER})



