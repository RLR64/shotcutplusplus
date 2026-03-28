cmake_minimum_required(VERSION 3.12...3.31)

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.20")
  cmake_policy(SET CMP0115 NEW)
endif()

if(NOT DEFINED SHOTCUT_VERSION)
  set(SHOTCUT_VERSION "26.2")
endif()

project(shotcut
  VERSION ${SHOTCUT_VERSION}
  DESCRIPTION "Shotcut++ - Windows video editor, modernized C++ fork"
  HOMEPAGE_URL "https://github.com/RLR64/shotcutplusplus"
  LANGUAGES CXX
)

if(WIN32)
  option(WINDOWS_DEPLOY "Install exes/libs directly to prefix (no subdir /bin)" ON)
endif()
option(CLANG_FORMAT "Enable Clang Format" ON)
option(EXTERNAL_LAUNCHERS "Whether include features to launch external programs; for example, this should be off for Flatpak due to sandbox." ON)
option(USE_VULKAN "Whether to use Vulkan for hardware video decoding" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(FeatureSummary)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 6.4 REQUIRED
  COMPONENTS
  Charts
  Multimedia
  Network
  OpenGL
  OpenGLWidgets
  QuickControls2
  QuickWidgets
  Sql
  WebSockets
  Widgets
  Xml
)

if(UNIX AND NOT APPLE)
  find_package(Qt6 6.4 REQUIRED COMPONENTS DBus)
  # X11 for WindowPicker (Linux/X11)
  find_package(X11 REQUIRED)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(mlt++ REQUIRED IMPORTED_TARGET mlt++-7>=7.36.0)
pkg_check_modules(FFTW IMPORTED_TARGET fftw3)
if(NOT FFTW_FOUND)
  pkg_check_modules(FFTW REQUIRED IMPORTED_TARGET fftw)
endif()

add_subdirectory(CuteLogger)
add_subdirectory(src)
add_subdirectory(translations)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

add_custom_target(codespell COMMAND
  cd "${CMAKE_SOURCE_DIR}" &&
  codespell -w -q 3
  -L shotcut,uint,seeked,really,prevent
  -S export-chapters.js,export-edl.js,CuteLogger,drmingw,node_modules,moc_*,index.theme,*.ts,three.min.js,three.js,jquery.js,rangy-*.js,Makefile,shotcut.pro,*.desktop
)

file(GLOB_RECURSE QML_SRC "src/qml/*.qml")
add_custom_target(qmlformat COMMAND
  ${CMAKE_PREFIX_PATH}/bin/qmlformat -i ${QML_SRC}
)
