include(FindRuby)

if(RUBY_EXECUTABLE)
  message(STATUS "Found ruby:	${RUBY_EXECUTABLE}")
endif()

message(STATUS "Looking for ruby.h")
if(RUBY_INCLUDE_DIR)
  message(STATUS "Looking for ruby.h - found")
else()
  message(STATUS "Looking for ruby.h - not found")
endif()

message(STATUS "Looking for confi.h")
if(RUBY_CONFIG_INCLUDE_DIR)
  message(STATUS "Looking for config.h - found")
else()
  message(STATUS "Looking for config.h - not found")
endif()

message(STATUS "Looking for lib ruby")
if(RUBY_LIBRARY)
  message(STATUS "Looking for lib ruby - found")
else()
  message(STATUS "Looking for lib ruby - not found")
endif()

if(RUBY_LIBRARY)
  set(LIB_RUBY_VERSION "${RUBY_VERSION_MAJOR}.${RUBY_VERSION_MINOR}.${RUBY_VERSION_PATCH}")
  message(STATUS "Lib ruby version: ${LIB_RUBY_VERSION}")
  if(RUBY_VERSION_MAJOR MATCHES "1" AND RUBY_VERSION_MINOR MATCHES "9")
    string(REGEX MATCH "ruby.*[0-9]" RUBY_LIBRARY_NAME ${RUBY_LIBRARY})
    if(NOT RUBY_LIBRARY_NAME)
      set(RUBY_LIBRARY_NAME ruby)
    endif()
    string(REGEX REPLACE "/libruby.*$" "" RUBY_LIBRARY ${RUBY_LIBRARY})
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}-I${RUBY_CONFIG_INCLUDE_DIR} ") #path to config.h
    string(COMPARE EQUAL "${RUBY_INCLUDE_DIR}" "${RUBY_CONFIG_INCLUDE_DIR}" operation)
    if(NOT operation)
      SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}-I${RUBY_INCLUDE_DIR} ") #path to ruby.h
    endif()
    ADD_DEFINITIONS("-I${CMAKE_HOME_DIRECTORY}/src/bindings/ruby -I${CMAKE_HOME_DIRECTORY}/src/simix")
    SET(HAVE_RUBY 1)
  else()
    message(STATUS "Warning: Ruby bindings need version 1.9.x, but found version ${RUBY_VERSION_MAJOR}.${RUBY_VERSION_MINOR}.x")
    SET(HAVE_RUBY 0)
  endif()
else()
  SET(HAVE_RUBY 0)
endif()

if(NOT RUBY_EXECUTABLE)
  message(STATUS "Warning: you are missing the ruby executable, so you can compile and build examples but can't execute them!")
endif()