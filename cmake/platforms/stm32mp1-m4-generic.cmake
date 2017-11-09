set (CMAKE_SYSTEM_PROCESSOR "arm" CACHE STRING "")
set (MACHINE "stm32mp1_m4" CACHE STRING "")

set (CMAKE_C_FLAGS          "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 " CACHE STRING "")

include (cross_generic_gcc)
