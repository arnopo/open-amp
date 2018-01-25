# Install script for directory: /local/home/frq07267/views/MCU_cube/mp1_v0.1.1/Firmware/Middlewares/Third_Party/OpenAMP/open-amp/lib

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/local/home/frq07267/views/MCU_cube/mp1_v0.1.1/Firmware/Middlewares/Third_Party/OpenAMP/open-amp/lib/libopen_amp.a")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "/local/home/frq07267/views/MCU_cube/mp1_v0.1.1/Firmware/Middlewares/Third_Party/OpenAMP/open-amp/lib/include/openamp")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/local/home/frq07267/views/MCU_cube/mp1_v0.1.1/Firmware/Middlewares/Third_Party/OpenAMP/open-amp/lib/common/cmake_install.cmake")
  include("/local/home/frq07267/views/MCU_cube/mp1_v0.1.1/Firmware/Middlewares/Third_Party/OpenAMP/open-amp/lib/virtio/cmake_install.cmake")
  include("/local/home/frq07267/views/MCU_cube/mp1_v0.1.1/Firmware/Middlewares/Third_Party/OpenAMP/open-amp/lib/rpmsg/cmake_install.cmake")
  include("/local/home/frq07267/views/MCU_cube/mp1_v0.1.1/Firmware/Middlewares/Third_Party/OpenAMP/open-amp/lib/remoteproc/cmake_install.cmake")
  include("/local/home/frq07267/views/MCU_cube/mp1_v0.1.1/Firmware/Middlewares/Third_Party/OpenAMP/open-amp/lib/proxy/cmake_install.cmake")

endif()

