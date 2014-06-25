set(kontactinterface_LIB_SRCS
    core.cpp
    plugin.cpp
    summary.cpp
    uniqueapphandler.cpp
    pimuniqueapplication.cpp)

add_library(KF5KontactInterface ${kontactinterface_LIB_SRCS})

generate_export_header(KF5KontactInterface BASE_NAME kontactinterface)

add_library(KF5::KontactInterface ALIAS KF5KontactInterface)

target_include_directories(KF5KontactInterface INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KontactInterface>")
target_include_directories(KF5KontactInterface PUBLIC "$<BUILD_INTERFACE:${KontactInterface_SOURCE_DIR}/src;${KontactInterface_BINARY_DIR}/src>")

target_link_libraries(KF5KontactInterface
                     PUBLIC
                      KF5::Parts
                      KF5::KDELibs4Support
                     PRIVATE 
                      KF5::PimUtils 
)

set_target_properties(KF5KontactInterface PROPERTIES
    VERSION ${KONTACTINTERFACE_VERSION_STRING}
    SOVERSION ${KONTACTINTERFACE_SOVERSION}
    EXPORT_NAME KontactInterface
)


install(TARGETS KF5KontactInterface EXPORT KF5KontactInterfaceTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

ecm_generate_headers(KontactInterface_CamelCase_HEADERS
  HEADER_NAMES
  Core
  PimUniqueApplication
  Plugin
  Summary
  UniqueAppHandler
  PREFIX KontactInterface
  REQUIRED_HEADERS KontactInterface_HEADERS
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kontactinterface_export.h
  ${KontactInterface_HEADERS}
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KontactInterface/kontactinterface
  COMPONENT Devel
)

install(FILES
    ${KontactInterface_CamelCase_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KontactInterface/KontactInterface/
    COMPONENT Devel
)


install(FILES kontactplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

ecm_generate_pri_file(BASE_NAME KontactInterface LIB_NAME KF5KontactInterface DEPS "Parts KDELibs4Support" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KF5_INCLUDE_INSTALL_DIR}/KontactInterface)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

