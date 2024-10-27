
file(
  GLOB
  headers
  *.hpp
  )

install(FILES ${headers}
  DESTINATION include/${CMAKE_PROJECT_NAME})


add_executable            (xrsw xrsw.cpp) 
set_target_properties     (xrsw PROPERTIES COMPILE_FLAGS "${PROJECT_CFLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}") 
target_link_libraries     (xrsw ${SKED_LIBRARIES})
target_include_directories(xrsw PUBLIC ${SKED_INCLUDE_DIRS})    
target_link_libraries     (xrsw ${ASIO_LIBRARIES})
target_include_directories(xrsw PUBLIC ${ASIO_INCLUDE_DIRS})    
install                   (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/xrsw RENAME ${CMAKE_PROJECT_NAME}-xrsw DESTINATION bin COMPONENT binary)
