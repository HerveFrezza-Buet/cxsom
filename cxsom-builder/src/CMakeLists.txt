
file(
  GLOB
  headers
  *.hpp
  )

install(FILES ${headers}
  DESTINATION include/${CMAKE_PROJECT_NAME})


