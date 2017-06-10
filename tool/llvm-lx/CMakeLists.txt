set(LLVM_USED_LIBS lx)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(llvm-lx
  main.cpp
)

llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES ${LLVM_TARGETS_TO_BUILD}
        asmparser core linker bitreader bitwriter irreader ipo scalaropts
        analysis target mc support)

target_link_libraries(llvm-lx lx ${REQ_LLVM_LIBRARIES})

# Platform dependencies.
#target_link_libraries(epp
    #pthread
    #dl
    #curses
#)

set_target_properties(llvm-lx
                      PROPERTIES
                      LINKER_LANGUAGE CXX
                      PREFIX "")

install(TARGETS llvm-lx 
  RUNTIME DESTINATION bin)

