
file(
  GLOB
  headers
  *.hpp
  )

install(FILES ${headers}
  DESTINATION include/${CMAKE_PROJECT_NAME})


add_executable            (xrsw xrsw.cpp) 
set_target_properties     (xrsw PROPERTIES COMPILE_FLAGS "${PROJECT_CFLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}") 
install                   (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/xrsw RENAME ${CMAKE_PROJECT_NAME}-xrsw DESTINATION bin COMPONENT binary)
