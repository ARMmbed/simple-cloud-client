INCLUDE(CMakeForceCompiler)

#This file defines needed options for native GCC compiler.


######### Configure all Options heere ##########
################################################
IF(NOT CMAKE_TOOLCHAIN_FILE_CONFIGURED)

SET(CMAKE_TOOLCHAIN_FILE_CONFIGURED  "1" CACHE INTERNAL "guard for toolchain file" FORCE)
# TOOLCHAIN EXTENSION
IF(WIN32)
    SET(TOOLCHAIN_EXT ".exe")
ELSE()
    SET(TOOLCHAIN_EXT "")
ENDIF()

# EXECUTABLE EXTENSION
SET (CMAKE_EXECUTABLE_SUFFIX "")


# TOOLCHAIN_DIR AND NANO LIBRARY
SET(TOOLCHAIN_DIR $ENV{GCC_DIR})
STRING(REGEX REPLACE "\\\\" "/" TOOLCHAIN_DIR "${TOOLCHAIN_DIR}")

IF(NOT TOOLCHAIN_DIR)
    MESSAGE("using /usr as basedir for toolchanin. Please set GCC_DIR if otherwise needed")
    set(TOOLCHAIN_DIR "/usr")
ENDIF()

MESSAGE(STATUS "TOOLCHAIN_DIR: " ${TOOLCHAIN_DIR})

# TARGET_TRIPLET - none in the case of native compilation
SET(TARGET_TRIPLET "")

SET(TOOLCHAIN_BIN_DIR ${TOOLCHAIN_DIR}/bin)
SET(TOOLCHAIN_INC_DIR ${TOOLCHAIN_DIR}/include)
SET(TOOLCHAIN_LIB_DIR ${TOOLCHAIN_DIR}/lib)

SET(CMAKE_SYSTEM_NAME Generic)
#SET(CMAKE_SYSTEM_PROCESSOR arm)

CMAKE_FORCE_C_COMPILER(${TOOLCHAIN_BIN_DIR}/gcc${TOOLCHAIN_EXT} GNU)
CMAKE_FORCE_CXX_COMPILER(${TOOLCHAIN_BIN_DIR}/g++${TOOLCHAIN_EXT} GNU)
SET(CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN_DIR}/gcc${TOOLCHAIN_EXT})

SET(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_DIR}/objcopy CACHE INTERNAL "objcopy tool")
# SET(CMAKE_OBJCOPY ${TOOLCHAIN_DIR}/${TARGET_TRIPLET}/bin/objcopy CACHE INTERNAL "objcopy tool")
SET(CMAKE_OBJDUMP ${TOOLCHAIN_BIN_DIR}/objdump CACHE INTERNAL "objdump tool")


macro(SET_COMPILER_DBG_RLZ_FLAG flag value)
    SET(${flag}_DEBUG "${${flag}_DEBUG} ${value}")
    SET(${flag}_RELEASE "${${flag}_RELEASE} ${value}")
#enable this if for debugging
if (0)
 message("flag = ${flag}")
 message("value = ${value}")
 message("MY_C_FLAGS_RELEASE2 = ${CMAKE_C_FLAGS_RELEASE}")
endif(0) # comment end
endmacro(SET_COMPILER_DBG_RLZ_FLAG)


SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g" CACHE INTERNAL "c compiler flags debug")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g" CACHE INTERNAL "cxx compiler flags debug")
SET(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG} -g" CACHE INTERNAL "asm compiler flags debug")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}" CACHE INTERNAL "linker flags debug")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 " CACHE INTERNAL "c compiler flags release")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 " CACHE INTERNAL "cxx compiler flags release")
SET(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_ASM_FLAGS_RELEASE}" CACHE INTERNAL "asm compiler flags release")
SET(CMAKE_EXE_LINKER_FLAGS_RELESE "${CMAKE_EXE_LINKER_FLAGS_RELESE}" CACHE INTERNAL "linker flags release")




########### COMPILER FLAGS  ###########
#
#######################################

########### DEBUG ###########

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-D__STARTUP_CLEAR_BSS")


SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-mcpu=${CPU}")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-Wall")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-mfloat-abi=hard")
# Floating point support

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-mthumb")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-fno-common")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-ffunction-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-fdata-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-ffreestanding")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-fno-builtin")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-mapcs")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-std=c99")


# Debug specific
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

# Board specific

# OS specific

# Board specific




SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-Wall")
# Board specific
if (${CPU} MATCHES "x86_64")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-m32")
endif ()
if (${CPU} MATCHES "cortex-m4")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-mcpu=${CPU}")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-mfloat-abi=hard")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-mapcs")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-mthumb")
endif()

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-MMD")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-MP")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-fno-common")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-ffunction-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-fdata-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-ffreestanding")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-fno-builtin")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-std=gnu99")

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "@${CMAKE_SOURCE_DIR}/include_file.txt")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "@${CMAKE_SOURCE_DIR}/include_file.txt")

########### Release specific ###########
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os")
######################################


########### LINKER FLAGS  ###########
#
#
#####################################

########### DEBUG ###########
# Debug specific
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g")


########### RELEASE ###########

if (${CPU} MATCHES "x86_64")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-m32")
elseif (${CPU} MATCHES "cortex-m4")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-mcpu=${CPU}")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-mfloat-abi=hard")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "--specs=nano.specs")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-mthumb")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-mapcs")
endif()

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Wall")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-fno-common")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-ffunction-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-fdata-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-ffreestanding")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-fno-builtin")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "--gc-sections")
#SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker")
#SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-static")
#SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker")
#SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-z")
#SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker")
#SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "muldefs")



#########
if (0)
SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR}/${TARGET_TRIPLET} ${EXTRA_FIND_PATH})
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif() # comment end
SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR}/${TARGET_TRIPLET} ${EXTRA_FIND_PATH})
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)


MESSAGE(STATUS "BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
ENDIF()
