IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(CURRENT_OS "linux")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    SET(CURRENT_OS "windows")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	SET(CURRENT_OS "macos")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "iOS")
	SET(CURRENT_OS "ios")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Android")
	SET(CURRENT_OS "android")
ELSE()
    MESSAGE(FATAL_ERROR "NOT SUPPORT THIS SYSTEM")
ENDIF()

# ---- vnn library configuration ----
# Configure the face detection and tracking library for all platforms
set(VNN_LIB_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/vnn/libs/${CURRENT_OS})
set(VNN_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/vnn/include PARENT_SCOPE)

if(GPUPIXEL_ENABLE_FACE_DETECTOR)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    add_library(vnn_kit SHARED IMPORTED)
    set_target_properties(vnn_kit PROPERTIES IMPORTED_LOCATION
    ${CMAKE_CURRENT_SOURCE_DIR}/vnn/libs/${CURRENT_OS}/${ANDROID_ABI}/libvnn_kit.so)

    add_library(vnn_core SHARED IMPORTED)
    set_target_properties(vnn_core PROPERTIES IMPORTED_LOCATION
    ${CMAKE_CURRENT_SOURCE_DIR}/vnn/libs/${CURRENT_OS}/${ANDROID_ABI}/libvnn_core.so)

    add_library(vnn_face SHARED IMPORTED)
    set_target_properties(vnn_face PROPERTIES IMPORTED_LOCATION
    ${CMAKE_CURRENT_SOURCE_DIR}/vnn/libs/${CURRENT_OS}/${ANDROID_ABI}/libvnn_face.so)
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    if(GPUPIXEL_ENABLE_VNN)
      add_library(vnn_kit SHARED IMPORTED GLOBAL)
      set_target_properties(vnn_kit PROPERTIES IMPORTED_LOCATION
      ${CMAKE_CURRENT_SOURCE_DIR}/vnn/libs/${CURRENT_OS}/x64/vnn_kit.dll
      IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/vnn/libs/${CURRENT_OS}/x64/vnn_kit.lib)

      add_library(vnn_core SHARED IMPORTED GLOBAL)
      set_target_properties(vnn_core PROPERTIES IMPORTED_LOCATION
      ${CMAKE_CURRENT_SOURCE_DIR}/vnn/libs/${CURRENT_OS}/x64/vnn_core.dll
      IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/vnn/libs/${CURRENT_OS}/x64/vnn_core.lib)

      add_library(vnn_face SHARED IMPORTED GLOBAL)
      set_target_properties(vnn_face PROPERTIES IMPORTED_LOCATION
      ${CMAKE_CURRENT_SOURCE_DIR}/vnn/libs/${CURRENT_OS}/x64/vnn_face.dll
      IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/vnn/libs/${CURRENT_OS}/x64/vnn_face.lib)
    endif()
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    if(GPUPIXEL_ENABLE_VNN)
      add_library(vnn_kit SHARED IMPORTED)
      set_target_properties(vnn_kit PROPERTIES IMPORTED_LOCATION
      ${CMAKE_CURRENT_SOURCE_DIR}/vnn/libs/${CURRENT_OS}/libvnn_kit.so)

      add_library(vnn_face SHARED IMPORTED)
      set_target_properties(vnn_face PROPERTIES IMPORTED_LOCATION
      ${CMAKE_CURRENT_SOURCE_DIR}/vnn/libs/${CURRENT_OS}/libvnn_face.so)

      add_library(vnn_core SHARED IMPORTED)
      set_target_properties(vnn_core PROPERTIES IMPORTED_LOCATION
      ${CMAKE_CURRENT_SOURCE_DIR}/vnn/libs/${CURRENT_OS}/libvnn_core.so)
    endif()

    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath,./")
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR ${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    if(GPUPIXEL_ENABLE_VNN)
      message(STATUS "Configuring vnn for ${VNN_LIB_DIRS}")
      find_library(vnn_kit_lib vnn_kit_osx PATHS ${VNN_LIB_DIRS} NO_DEFAULT_PATH)
      find_library(vnn_face_lib vnn_face_osx PATHS ${VNN_LIB_DIRS} NO_DEFAULT_PATH)
      find_library(vnn_core_lib vnn_core_osx PATHS ${VNN_LIB_DIRS} NO_DEFAULT_PATH)
      if(NOT vnn_kit_lib)
        message(FATAL_ERROR "vnn_kit library not found in ${VNN_LIB_DIRS}")
      endif()

      add_library(vnn_kit SHARED IMPORTED GLOBAL)
      set_target_properties(vnn_kit PROPERTIES IMPORTED_LOCATION ${vnn_core_lib})
      add_library(vnn_core SHARED IMPORTED GLOBAL)
      set_target_properties(vnn_core PROPERTIES IMPORTED_LOCATION ${vnn_kit_lib})
      add_library(vnn_face SHARED IMPORTED GLOBAL)
      set_target_properties(vnn_face PROPERTIES IMPORTED_LOCATION ${vnn_face_lib})
    endif()
  endif()

  # ---- Runtime library installation ----
  # Install shared libraries that are needed at runtime

  # ---- MNN library configuration ----
  # Neural network acceleration framework (Apple platforms only)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_library(mnn STATIC IMPORTED GLOBAL)
    set(mnn_lib_path ${CMAKE_CURRENT_SOURCE_DIR}/mnn/libs/macos)

    # find library file
    find_library(
      mnn_lib
      NAMES "MNN"
      PATHS ${mnn_lib_path}
      NO_DEFAULT_PATH)

    set_target_properties(mnn PROPERTIES IMPORTED_LOCATION ${mnn_lib})
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    add_library(mnn STATIC IMPORTED GLOBAL)
    set(mnn_lib_path ${CMAKE_CURRENT_SOURCE_DIR}/mnn/libs/ios)

    # find library file
    find_library(
      mnn_lib
      NAMES "MNN"
      PATHS ${mnn_lib_path}
      NO_DEFAULT_PATH)

    set_target_properties(mnn PROPERTIES IMPORTED_LOCATION ${mnn_lib})
  endif()
endif()
# ---- libyuv configuration ----
# YUV image processing library for efficient color space conversion
add_subdirectory(libyuv EXCLUDE_FROM_ALL) # EXCLUDE_FROM_ALL means don't install
                                          # yuv target

add_library(libyuv::yuv ALIAS yuv)

target_include_directories(
  yuv PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libyuv/include>)

# add -fPIC option to solve dynamic library linking issues
set_target_properties(yuv PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  install(
    TARGETS yuv
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)
endif()

# ---- Desktop platform dependencies ----
# Libraries specific to desktop platforms (macOS, Windows, Linux)
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin"
   OR ${CMAKE_SYSTEM_NAME} MATCHES "Windows"
   OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  # ---- glad configuration ----
  # OpenGL loader library
  add_library(glad STATIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/src/glad.c)

  add_library(glad::glad ALIAS glad)

  # add -fPIC option to solve dynamic library linking issues
  set_target_properties(glad PROPERTIES POSITION_INDEPENDENT_CODE ON)

  target_include_directories(
    glad PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/glad/include>)

  # ---- glfw configuration ----
  # Cross-platform window and input management
  set(GLFW_BUILD_EXAMPLES
      OFF
      CACHE BOOL "Disable building GLFW examples")
  set(GLFW_BUILD_TESTS
      OFF
      CACHE BOOL "Disable building GLFW tests")
  set(GLFW_INSTALL
      OFF
      CACHE BOOL "Disable GLFW installation")
  add_subdirectory(glfw EXCLUDE_FROM_ALL)

  add_library(glfw::glfw ALIAS glfw)

  if(APPLE)
    # disable ARC for Objective-C and Objective-C++ files
    target_compile_options(glfw PRIVATE "-fno-objc-arc")
  endif()
endif()

# ---- stb image library configuration ----
# Header-only image loading/saving library
add_library(stb INTERFACE)

target_include_directories(
  stb INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/stb/include>)

add_library(stb::stb ALIAS stb)

# ---- ghc_filesystem configuration ----
# Header-only filesystem library
add_library(ghc_filesystem INTERFACE)

add_library(ghc::filesystem ALIAS ghc_filesystem)

target_include_directories(
  ghc_filesystem
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ghc/include>)
