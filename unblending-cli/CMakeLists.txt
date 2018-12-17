file(GLOB files *.cpp *.h)
add_executable(unblending-cli ${files})
target_link_libraries(unblending-cli unblending cxxopts)
