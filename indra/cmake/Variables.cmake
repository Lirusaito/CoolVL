# -*- cmake -*-
#
# Definitions of variables used throughout the Second Life build
# process.
#
# Platform variables:
#
#   DARWIN  - Mac OS X
#   LINUX   - Linux
#   WINDOWS - Windows
#
# What to build:
#
#   VIEWER - viewer and other viewer-side components


# Relative and absolute paths to subtrees.

set(LIBS_CLOSED_PREFIX)
set(LIBS_OPEN_PREFIX)
set(SCRIPTS_PREFIX ../scripts)
set(VIEWER_PREFIX)

set(LIBS_CLOSED_DIR ${CMAKE_SOURCE_DIR}/${LIBS_CLOSED_PREFIX})
set(LIBS_OPEN_DIR ${CMAKE_SOURCE_DIR}/${LIBS_OPEN_PREFIX})
set(SCRIPTS_DIR ${CMAKE_SOURCE_DIR}/${SCRIPTS_PREFIX})
set(VIEWER_DIR ${CMAKE_SOURCE_DIR}/${VIEWER_PREFIX})

set(LIBS_PREBUILT_DIR ${CMAKE_SOURCE_DIR}/../libraries CACHE PATH
    "Location of prebuilt libraries.")

if (EXISTS ${CMAKE_SOURCE_DIR}/Server.cmake)
  # We use this as a marker that you can try to use the proprietary libraries.
  set(INSTALL_PROPRIETARY ON CACHE BOOL "Install proprietary binaries")
endif (EXISTS ${CMAKE_SOURCE_DIR}/Server.cmake)


if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(WINDOWS ON BOOL FORCE)
  set(ARCH i686)
  set(LL_ARCH ${ARCH}_win32)
  set(LL_ARCH_DIR ${ARCH}-win32)
  set(WORD_SIZE 32)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LINUX ON BOOl FORCE)

  # If someone has specified a word size, use that to determine the
  # architecture.  Otherwise, let the architecture specify the word size.
  if (WORD_SIZE EQUAL 32)
    set(ARCH i686)
  elseif (WORD_SIZE EQUAL 64)
    set(ARCH x86_64)
  else (WORD_SIZE EQUAL 32)
    if(CMAKE_SIZEOF_VOID_P MATCHES 4)
      set(ARCH i686)
      set(WORD_SIZE 32)
    else(CMAKE_SIZEOF_VOID_P MATCHES 4)
      set(ARCH x86_64)
      set(WORD_SIZE 64)
    endif(CMAKE_SIZEOF_VOID_P MATCHES 4)
  endif (WORD_SIZE EQUAL 32)

  set(LL_ARCH ${ARCH}_linux)
  set(LL_ARCH_DIR ${ARCH}-linux)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(DARWIN 1)
  # set this dynamically from the build system now -
  # NOTE: wont have a distributable build unless you add this on the configure line with:
  # -DCMAKE_OSX_ARCHITECTURES:STRING='i386;ppc'
  set(CMAKE_OSX_ARCHITECTURES i386) 

  # To support a different SDK update these Xcode settings:
  set(CMAKE_OSX_DEPLOYMENT_TARGET 10.5)
  set(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX10.5.sdk)
  set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "4.0")
  # set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT dwarf-with-dsym)
  if (CMAKE_OSX_ARCHITECTURES MATCHES "i386")
    set(ARCH i386)
  endif (CMAKE_OSX_ARCHITECTURES MATCHES "i386")

  set(LL_ARCH ${ARCH}_darwin)
  set(LL_ARCH_DIR universal-darwin)
  set(WORD_SIZE 32)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# Default deploy grid
set(GRID agni CACHE STRING "Target Grid")

set(VIEWER ON CACHE BOOL "Build Second Life viewer.")
set(VIEWER_CHANNEL "CommunityDeveloper" CACHE STRING "Viewer Channel Name")
set(VIEWER_LOGIN_CHANNEL ${VIEWER_CHANNEL} CACHE STRING "Fake login channel for A/B Testing")
set(VIEWER_BRANDING_ID "cool_vl_viewer" CACHE STRING "Viewer branding id (currently cool_vl_viewer)")
set(VIEWER_BRANDING_NAME "Cool VL Viewer")
set(VIEWER_BRANDING_NAME_CAMELCASE "CoolVLViewer")

set(STANDALONE OFF CACHE BOOL "Do not use Linden-supplied prebuilt libraries.")

source_group("CMake Rules" FILES CMakeLists.txt)
