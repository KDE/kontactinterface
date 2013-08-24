project(kontactinterface)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5601)

include_directories(${CMAKE_SOURCE_DIR}/kpimutils)

set(kontactinterface_LIB_SRCS
    core.cpp
    plugin.cpp
    summary.cpp
    uniqueapphandler.cpp
    pimuniqueapplication.cpp)

kde4_add_library(kontactinterface ${LIBRARY_TYPE} ${kontactinterface_LIB_SRCS})

target_link_libraries(kontactinterface kpimutils ${KDE4_KPARTS_LIBS})

set_target_properties(kontactinterface PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS kontactinterface EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
  kontactinterface_export.h
  core.h
  plugin.h
  summary.h
  uniqueapphandler.h
  pimuniqueapplication.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kontactinterface COMPONENT Devel)

install(FILES kontactplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

