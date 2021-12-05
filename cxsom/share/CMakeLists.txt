
file(
  GLOB shared
  cxsom-makefile
  )

install(FILES ${shared}
  DESTINATION share/${CMAKE_PROJECT_NAME})
