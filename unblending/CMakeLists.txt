find_package(Eigen3 REQUIRED)
if((NOT TARGET Eigen3::Eigen) AND (DEFINED EIGEN3_INCLUDE_DIR))
	# Eigen 3.0, 3.1, 3.2 do not provide the target named Eigen3::Eigen
	add_library(AliasEigen3 INTERFACE)
	target_include_directories(AliasEigen3 INTERFACE ${EIGEN3_INCLUDE_DIR})
	add_library(Eigen3::Eigen ALIAS AliasEigen3)
endif()

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
