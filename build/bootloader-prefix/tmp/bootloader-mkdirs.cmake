# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/esp/v4.4/esp-idf/components/bootloader/subproject"
  "C:/Users/teunl/Documents/school/Jaar-2/Periode-3/Project/Product/smartspeaker_a1/build/bootloader"
  "C:/Users/teunl/Documents/school/Jaar-2/Periode-3/Project/Product/smartspeaker_a1/build/bootloader-prefix"
  "C:/Users/teunl/Documents/school/Jaar-2/Periode-3/Project/Product/smartspeaker_a1/build/bootloader-prefix/tmp"
  "C:/Users/teunl/Documents/school/Jaar-2/Periode-3/Project/Product/smartspeaker_a1/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/teunl/Documents/school/Jaar-2/Periode-3/Project/Product/smartspeaker_a1/build/bootloader-prefix/src"
  "C:/Users/teunl/Documents/school/Jaar-2/Periode-3/Project/Product/smartspeaker_a1/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/teunl/Documents/school/Jaar-2/Periode-3/Project/Product/smartspeaker_a1/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
