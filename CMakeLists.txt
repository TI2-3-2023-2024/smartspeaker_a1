# The following four lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
set(EXTRA_COMPONENT_DIRS C:/esp/esp-adf/components)
set(EXTRA_COMPONENT_DIRS C:/esp/external_libs/esp-idf-lib-v0.9.4/components)

include($ENV{ADF_PATH}/CMakeLists.txt)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(example-hd44780-i2c)
