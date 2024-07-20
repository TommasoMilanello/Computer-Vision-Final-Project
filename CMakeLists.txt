#AUTHOR: Tommaso Milanello

cmake_minimum_required(VERSION 3.10)

# Project name and version
project(CV_Final_Project VERSION 1.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Add the executable
add_executable(CV_Final_Project main.cpp BallDetection.cpp BBox.cpp DisplayFunctions.cpp MiniMap.cpp SegmentationEvaluation.cpp TableDetection.cpp SegmentationEvaluation.cpp ReportUtilitiesFunctions.cpp BallClassification.cpp MainUtilities.cpp)

# Include directories
target_include_directories(CV_Final_Project PRIVATE ${OpenCV_INCLUDE_DIRS})

# Link libraries
target_link_libraries(CV_Final_Project PRIVATE ${OpenCV_LIBS})

# Copy the image file to the build directory
file(COPY ${CMAKE_SOURCE_DIR}/map_background.png DESTINATION ${CMAKE_BINARY_DIR})

