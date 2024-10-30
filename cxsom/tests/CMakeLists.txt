
file(
  GLOB
  usage_tests
  test-*.cpp
)

foreach(f ${usage_tests})
  get_filename_component(testName ${f} NAME_WE)
  add_executable            (${testName}                            ${f}                     )
  set_target_properties     (${testName} PROPERTIES LINKER_LANGUAGE CXX                      )
  set_target_properties     (${testName} PROPERTIES COMPILE_FLAGS   "${PROJECT_CFLAGS} -Wall")
  
  target_include_directories(${testName} PUBLIC ${FFTCONV_INCLUDE_DIRS})    
  target_include_directories(${testName} PUBLIC ${SKEDNET_INCLUDE_DIRS})    
  target_include_directories(${testName} PUBLIC ${CMAKE_SOURCE_DIR}/src)
  target_link_libraries     (${testName} PUBLIC ${FFTCONV_LIBRARIES} ${SKEDNET_LIBRARIES} pthread)
  
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${testName}
    DESTINATION bin
    RENAME ${CMAKE_PROJECT_NAME}-${testName}
    COMPONENT binary)
endforeach(f)
