find_package(Qt5 COMPONENTS Widgets Concurrent REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB files *.cpp *.h *.ui)
if (APPLE) 
	add_executable(unblending-gui MACOSX_BUNDLE ${files})
	set_target_properties(unblending-gui PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../resources/Info.plist.in)
else()
	add_executable(unblending-gui ${files})	
endif()
target_link_libraries(unblending-gui Qt5::Widgets Qt5::Concurrent unblending)
