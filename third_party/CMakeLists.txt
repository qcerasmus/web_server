cmake_minimum_required(VERSION 2.8.2)

include(ExternalProject)
ExternalProject_Add(asio
  GIT_REPOSITORY    https://github.com/chriskohlhoff/asio.git
  GIT_TAG           asio-1-18-0
  SOURCE_DIR        "${CMAKE_SOURCE_DIR}/third_party/asio"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)