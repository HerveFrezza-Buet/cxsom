
file(
  GLOB
  usage_examples
  example-*.cpp
)

foreach(f ${usage_examples})
  get_filename_component(exampleName ${f} NAME_WE)
  add_executable            (${exampleName}                            ${f}                     )
  set_target_properties     (${exampleName} PROPERTIES LINKER_LANGUAGE CXX                      )
  set_target_properties     (${exampleName} PROPERTIES COMPILE_FLAGS   "${PROJECT_CFLAGS} -Wall")
  
  target_include_directories(${exampleName} PUBLIC ${CMAKE_SOURCE_DIR}/src)
  target_include_directories(${exampleName} PUBLIC ${SKED_INCLUDE_DIRS})   
  target_include_directories(${exampleName} PUBLIC ${ASIO_INCLUDE_DIRS})   
  target_link_libraries     (${exampleName} PUBLIC -lpthread)   
  target_link_libraries     (${exampleName} PUBLIC ${SKED_LIBRARIES})  
  target_link_libraries     (${exampleName} PUBLIC ${ASIO_LIBRARIES})  

  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${exampleName}
    DESTINATION bin
    RENAME ${CMAKE_PROJECT_NAME}-${exampleName}
    COMPONENT binary)
endforeach(f)
