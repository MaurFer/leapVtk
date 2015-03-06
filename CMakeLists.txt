cmake_minimum_required(VERSION 2.8)

PROJECT(leapVtk)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

set(leapdir /home/sterling/Desktop/hackathon/LeapDeveloperKit_2.2.3+25971_linux/LeapSDK )
include_directories(${leapdir}/include)
set(leapHeader ${leapdir}/include/Leap.h)



add_executable(leapVtk MACOSX_BUNDLE leapVtk ${leapHeader})


target_link_libraries(leapVtk ${VTK_LIBRARIES} ${leapdir}/lib/x64/libLeap.so)

