
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
target_link_libraries     (processor ${FFTCONV_LIBRARIES} -lpthread)
install                   (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/processor RENAME ${CMAKE_PROJECT_NAME}-processor DESTINATION bin COMPONENT binary)

add_executable            (verbose-processor processor.cpp) 
target_include_directories(verbose-processor PUBLIC ${FFTCONV_INCLUDE_DIRS})    
set_target_properties     (verbose-processor PROPERTIES COMPILE_FLAGS "${PROJECT_CFLAGS} -I${CMAKE_CURRENT_SOURCE_DIR} -DcxsomLOG") 
target_link_libraries     (verbose-processor ${FFTCONV_LIBRARIES} -lpthread)
install                   (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/verbose-processor RENAME ${CMAKE_PROJECT_NAME}-verbose-processor DESTINATION bin COMPONENT binary)

add_executable            (monitored-processor processor.cpp) 
target_include_directories(monitored-processor PUBLIC ${FFTCONV_INCLUDE_DIRS})    
set_target_properties     (monitored-processor PROPERTIES COMPILE_FLAGS "${PROJECT_CFLAGS} -I${CMAKE_CURRENT_SOURCE_DIR} -DcxsomMONITOR") 
target_link_libraries     (monitored-processor ${FFTCONV_LIBRARIES} -lpthread)
install                   (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/monitored-processor RENAME ${CMAKE_PROJECT_NAME}-monitored-processor DESTINATION bin COMPONENT binary)

add_executable            (protocol-processor processor.cpp) 
target_include_directories(protocol-processor PUBLIC ${FFTCONV_INCLUDE_DIRS})    
set_target_properties     (protocol-processor PROPERTIES COMPILE_FLAGS "${PROJECT_CFLAGS} -I${CMAKE_CURRENT_SOURCE_DIR} -DcxsomDEBUG_PROTOCOL") 
target_link_libraries     (protocol-processor ${FFTCONV_LIBRARIES} -lpthread)
install                   (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/protocol-processor RENAME ${CMAKE_PROJECT_NAME}-protocol-processor DESTINATION bin COMPONENT binary)

add_executable            (ping ping.cpp) 
target_link_libraries     (ping -lpthread)
install                   (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ping RENAME ${CMAKE_PROJECT_NAME}-ping DESTINATION bin COMPONENT binary)

add_executable            (clear clear.cpp) 
target_link_libraries     (clear -lpthread)
install                   (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/clear RENAME ${CMAKE_PROJECT_NAME}-clear DESTINATION bin COMPONENT binary)

add_executable            (ask ask.cpp) 
install                   (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ask RENAME ${CMAKE_PROJECT_NAME}-ask DESTINATION bin COMPONENT binary)

add_executable            (conditional-message conditional-message.cpp) 
install                   (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/conditional-message RENAME ${CMAKE_PROJECT_NAME}-conditional-message DESTINATION bin COMPONENT binary)
