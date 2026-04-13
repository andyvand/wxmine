cmake_minimum_required(VERSION 3.14)
project(wxmine C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Locate wxWidgets. On macOS/Linux this uses wx-config via the CMake FindwxWidgets
# module; on Windows it resolves from wxWidgets_ROOT_DIR or WXWIN env var.
find_package(wxWidgets REQUIRED COMPONENTS xrc html adv core base xml)

if(NOT WIN32)
    include(${wxWidgets_USE_FILE})
endif()

# Locate the wxrc resource compiler (used to package the XRC into a .xrs zip).
if(NOT wxWidgets_wxrc_EXECUTABLE)
    find_program(wxWidgets_wxrc_EXECUTABLE
                 NAMES wxrc wxrc-3.2 wxrc-3.0
                 HINTS ${wxWidgets_ROOT_DIR}/bin
                       ${wxWidgets_LIB_DIR}/../bin)
endif()

if(NOT wxWidgets_wxrc_EXECUTABLE)
    message(FATAL_ERROR "wxrc not found. Install the wxWidgets development tools.")
endif()

# Inputs to the .xrs build: the XRC file plus the PNG/WAV/ICO assets that the
# C++ runtime will pull out of the archive at runtime via wxArchiveFSHandler.
set(XRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/xrc/winmine.xrc)
file(GLOB XRC_ASSETS
     ${CMAKE_CURRENT_SOURCE_DIR}/assets/*.png
     ${CMAKE_CURRENT_SOURCE_DIR}/assets/*.wav
     ${CMAKE_CURRENT_SOURCE_DIR}/assets/*.ico)

# Build bin2c host tool (preserved for non-XRC binary embedding tasks).
set(BIN2C ${CMAKE_BINARY_DIR}/bin2c)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(HostCompile)
host_compile(${CMAKE_CURRENT_SOURCE_DIR}/bin2c.c ${BIN2C})

# Build winmine.xrs:
#   1. wxrc -o winmine.xrs winmine.xrc — packages the XRC into a zip.
#   2. zip <xrs> <asset paths> — appends the PNG/WAV blobs at "assets/<name>"
#      so the runtime can OpenFile("file:.../winmine.xrs#zip:assets/<name>").
# The two-step is needed because wxrc only auto-includes files that the XRC
# directly references via <bitmap>/<icon>/etc. — our sprite sheets and wave
# files are loaded from C++ rather than from XRC, so wxrc skips them.
set(XRS_FILE ${CMAKE_CURRENT_BINARY_DIR}/winmine.xrs)

# Compute the asset paths relative to wx/ so that zip stores them as
# "assets/blocks.png" rather than as absolute paths.
set(XRS_ASSET_RELPATHS)
foreach(_asset ${XRC_ASSETS})
    file(RELATIVE_PATH _rel ${CMAKE_CURRENT_SOURCE_DIR} ${_asset})
    list(APPEND XRS_ASSET_RELPATHS ${_rel})
endforeach()

add_custom_command(
    OUTPUT ${XRS_FILE}
    COMMAND ${CMAKE_COMMAND} -E remove -f ${XRS_FILE}
    COMMAND ${wxWidgets_wxrc_EXECUTABLE}
            -o ${XRS_FILE}
            ${XRC_FILE}
    COMMAND zip -q ${XRS_FILE} ${XRS_ASSET_RELPATHS}
    DEPENDS ${XRC_FILE} ${XRC_ASSETS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Packaging XRC + assets into winmine.xrs"
    VERBATIM)

# Embed winmine.xrs as a C array via bin2c so the runtime can mount it
# under wxMemoryFSHandler at "memory:winmine.xrs" — no external file required.
set(XRS_HEADER ${CMAKE_CURRENT_BINARY_DIR}/winmine_xrs.h)

add_custom_command(
    OUTPUT ${XRS_HEADER}
    COMMAND ${BIN2C} ${XRS_FILE} ${XRS_HEADER} winmine_xrs
    DEPENDS ${XRS_FILE} ${BIN2C}
    COMMENT "Embedding winmine.xrs as C array via bin2c"
    VERBATIM)

set(SOURCES
    src/WinMineApp.cpp
    src/MainFrame.cpp
    src/BoardPanel.cpp
    src/GameCore.cpp
    src/Renderer.cpp
    src/Preferences.cpp
    src/SoundMgr.cpp
    src/PrefDialog.cpp
    src/BestDialog.cpp
    src/EnterDialog.cpp
    src/Resources.cpp)

# macOS app icon: list the .icns as a bundle resource so it lands in
# wxmine.app/Contents/Resources/, and set MACOSX_BUNDLE_ICON_FILE so CMake
# writes <key>CFBundleIconFile</key><string>wxmine.icns</string> into the
# auto-generated Info.plist.
set(MACOSX_ICNS ${CMAKE_CURRENT_SOURCE_DIR}/assets/wxmine.icns)
if(APPLE)
    set_source_files_properties(${MACOSX_ICNS} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources)
    list(APPEND SOURCES ${MACOSX_ICNS})
endif()

# Windows app icon + version info: the resource compiler picks up wxmine.rc,
# which #includes wx/msw/wx.rc (manifest + wx internals) and declares the
# app icon (assets/wxmine.ico). wxWidgets' find module sets
# wxWidgets_DEFINITIONS and wxWidgets_INCLUDE_DIRS that the RC compiler needs
# in order for #include "wx/msw/wx.rc" to resolve.
if(WIN32)
    list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/wxmine.rc)
endif()

add_executable(wxmine WIN32 MACOSX_BUNDLE ${SOURCES})

if(WIN32)
    # The RC compiler needs wxWidgets' include dirs so that #include
    # "wx/msw/wx.rc" resolves to the installed wx headers.
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/wxmine.rc
        PROPERTIES LANGUAGE RC)
    target_include_directories(wxmine PRIVATE ${wxWidgets_INCLUDE_DIRS})
endif()

# Resources.cpp is the single TU that #includes winmine_xrs.h, so depend it on
# the generated header explicitly.
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Resources.cpp
    PROPERTIES OBJECT_DEPENDS ${XRS_HEADER})

# Tell the C++ side where the bin2c header lives.
target_include_directories(wxmine PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(wxmine PRIVATE ${wxWidgets_LIBRARIES})

# macOS bundle niceties
if(APPLE)
    set_target_properties(wxmine PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
        MACOSX_BUNDLE_BUNDLE_NAME "wxmine"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.wxmine.app"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        MACOSX_BUNDLE_ICON_FILE "wxmine.icns")

elseif(UNIX)
    install(TARGETS wxmine RUNTIME DESTINATION bin)
    install(FILES     ${CMAKE_CURRENT_SOURCE_DIR}/wxmine.desktop      DESTINATION share/applications)
    install(FILES     ${CMAKE_CURRENT_SOURCE_DIR}/wxmine.metainfo.xml DESTINATION share/metainfo)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/sizes/                    DESTINATION share/icons/hicolor)

endif()
