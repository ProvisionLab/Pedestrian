cmake_minimum_required(VERSION 3.2)
project(darknet C CXX)

set(CMAKE_CXX_STANDARD 11)
set(GPU ON)

if(${GPU})
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
endif()
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)

include_directories(${SRC_DIR})

file(GLOB SRC_FILES ${SRC_DIR}/*.c)

list(REMOVE_ITEM SRC_FILES "${SRC_DIR}/darknet.c")
list(REMOVE_ITEM SRC_FILES "${SRC_DIR}/yolo_demo.c")

list(REMOVE_ITEM SRC_FILES "${SRC_DIR}/cpu_gemm.c")
list(REMOVE_ITEM SRC_FILES "${SRC_DIR}/server.c")
list(REMOVE_ITEM SRC_FILES "${SRC_DIR}/xnor_layer.c")
list(REMOVE_ITEM SRC_FILES "${SRC_DIR}/deconvolutional_layer.c")

#Threads
find_package(Threads REQUIRED)
list(APPEND LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

#CUDA
set(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
if(${GPU})
set(CUDA_USE_STATIC_CUDA_RUNTIME OFF)

find_package(CUDA REQUIRED)

message(STATUS "CUDA Version: " ${CUDA_VERSION_STRINGS})
message(STATUS "CUDA Libararies: " ${CUDA_LIBRARIES})

include_directories(SYSTEM ${CUDA_INCLUDE_DIRS})

list(APPEND LIBRARIES ${CUDA_LIBRARIES} ${CUDA_CUBLAS_LIBRARIES} ${CUDA_curand_LIBRARY} ${CUDA_cusparse_LIBRARY})
list(APPEND CUDA_NVCC_FLAGS "-arch=sm_35;-O2;-Xcompiler;-fPIC;")
list(APPEND CUDA_NVCC_FLAGS "-std=c++11;")
list(APPEND CUDA_NVCC_FLAGS  "-Xcompiler;-fPIC;")
list(APPEND CUDA_NVCC_FLAGS "--ftz=true;--prec-div=false;--prec-sqrt=false")

set(CUDA_ARCH_BIN "20 30 32 35 37 50 52" CACHE STRING "Specify 'real' GPU arch to build binaries for, BIN(PTX) format is supported. Example: 1.3 2.1(1.3) or 13 21(13)")
set(CUDA_ARCH_PTX "" CACHE STRING "Specify 'virtual' PTX arch to build PTX intermediate code for. Example: 1.0 1.2 or 10 12")

add_definitions(-DGPU)
cuda_include_directories(${SRC_DIR})

file(GLOB CU_SRC_FILES ${SRC_DIR}/*.cu)

list(REMOVE_ITEM CU_SRC_FILES "${SRC_DIR}/deconvolutional_kernels.cu")

include(CudaComputeTargetFlags)
APPEND_TARGET_ARCH_FLAGS()

cuda_compile(cuda_objs ${CU_SRC_FILES})
endif()

#OpenCV
find_package(OpenCV REQUIRED)
add_definitions(-DOPENCV)
include_directories(SYSTEM ${OpenCV_INCLUDE_DIR})

if(${GPU})
    add_executable(${PROJECT_NAME} ${SRC_DIR}/darknet.c ${SRC_FILES} ${CU_SRC_FILES} ${cuda_objs})
    target_link_libraries(${PROJECT_NAME} ${LIBRARIES} ${OpenCV_LIBS})
else()
    add_executable(yolo_demo ${SRC_DIR}/yolo_demo.c ${SRC_FILES})
    target_link_libraries(yolo_demo ${LIBRARIES} ${OpenCV_LIBS})
endif()