find_package(Eigen3 3.3 REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Threads REQUIRED)

file(GLOB headers include/unblending/*.hpp)
file(GLOB sources src/*.cpp)
add_library(unblending STATIC ${headers} ${sources})
target_link_libraries(unblending Eigen3::Eigen Qt5::Gui nlopt Threads::Threads json11 tinycolormap timer parallel-util nlopt-util)
target_include_directories(
	unblending
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

install(FILES ${headers} DESTINATION include/unblending)
install(TARGETS unblending ARCHIVE DESTINATION lib)
