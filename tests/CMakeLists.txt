cmake_minimum_required(VERSION 3.15)

# Test executable
add_executable(test_memory test_memory.cpp)

target_link_libraries(test_memory
    optistream_core
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER_APP_LIBRARIES}
    ${OpenCV_LIBS}
)

# Add test
add_test(NAME MemoryTest COMMAND test_memory)
