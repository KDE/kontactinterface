remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
include_directories(${CMAKE_SOURCE_DIR}/kpimutils)

set(kontactinterface_LIB_SRCS
    core.cpp
    plugin.cpp
    summary.cpp
    uniqueapphandler.cpp
    pimuniqueapplication.cpp)

include_directories(
  ${CMAKE_BINARY_DIR}/kpimutils
)

add_library(KF5KontactInterface ${kontactinterface_LIB_SRCS})

generate_export_header(KF5KontactInterface BASE_NAME kontactinterface)

add_library(KF5::KontactInterface ALIAS KF5KontactInterface)

target_include_directories(KF5KontactInterface INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/ktnef;${Boost_INCLUDE_DIR}>")
target_include_directories(KF5KontactInterface PUBLIC "$<BUILD_INTERFACE:${KontactInterface_SOURCE_DIR}/src;${KontactInterface_BINARY_DIR}/src>")

target_link_libraries(KF5KontactInterface 
                      KF5::PimUtils 
                      KF5::Parts 
                      KF5::KDE4Support
)

set_target_properties(KF5KontactInterface PROPERTIES
    VERSION ${KTNEF_VERSION_STRING}
    SOVERSION ${KTNEF_SOVERSION}
    EXPORT_NAME KontactInterface
)


install(TARGETS KF5KontactInterface EXPORT KF5KontactInterfaceTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kontactinterface_export.h
  core.h
  plugin.h
  summary.h
  uniqueapphandler.h
  pimuniqueapplication.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kontactinterface COMPONENT Devel)

install(FILES kontactplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

