cmake_minimum_required(VERSION 3.7)

project(vtp C)

#add_library(vtp_objects OBJECT virt_to_phys.c)
#set_source_files_properties(virt_to_phys.c PROPERTIES COMPILE_FLAGS -fPIC)

add_library(vtp SHARED virt_to_phys.c) #$<TARGET_OBJECTS:vtp_objects>)
target_include_directories(vtp PUBLIC ${CMAKE_CURRENT_LIST_DIR})
