
file(
  GLOB
  headers
  *.hpp
  )

install(FILES ${headers}
  DESTINATION include/${CMAKE_PROJECT_NAME})

add_executable            (all-instances all-instances.cpp) 
set_target_properties     (all-instances PROPERTIES COMPILE_FLAGS "${PROJECT_CFLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}") 
target_link_libraries     (all-instances ${FFTCONV_LIBRARIES})
target_include_directories(all-instances PUBLIC ${FFTCONV_INCLUDE_DIRS})    
install                   (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/all-instances RENAME ${CMAKE_PROJECT_NAME}-all-instances DESTINATION bin COMPONENT binary)

add_executable            (processor processor.cpp) 
target_include_directories(processor PUBLIC ${FFTCONV_INCLUDE_DIRS})    
set_target_properties     (processor PROPERTIES COMPILE_FLAGS "${PROJECT_CFLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}") 
target_link_libraries     (processor ${Boost_SYSTEM_LIBRARY} ${FFTCONV_LIBRARIES} -lpthread)
install                   (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/processor RENAME ${CMAKE_PROJECT_NAME}-processor DESTINATION bin COMPONENT binary)

add_executable            (ping ping.cpp) 
target_link_libraries     (ping ${Boost_SYSTEM_LIBRARY} -lpthread)
install                   (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ping RENAME ${CMAKE_PROJECT_NAME}-ping DESTINATION bin COMPONENT binary)

