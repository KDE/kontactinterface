add_library(KF5KontactInterface)
add_library(KF5::KontactInterface ALIAS KF5KontactInterface)
set(KF5KontactInterface
    core.cpp
    plugin.cpp
    summary.cpp
    processes.cpp
    uniqueapphandler.cpp
    pimuniqueapplication.cpp)

ecm_qt_declare_logging_category(KF5KontactInterface HEADER kontactinterface_debug.h IDENTIFIER KONTACTINTERFACE_LOG CATEGORY_NAME org.kde.pim.kontactinterface
        OLD_CATEGORY_NAMES log_kontactinterface
        DESCRIPTION "kontactinterface (pim lib)" EXPORT KONTACTINTERFACE)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KF5KontactInterface PROPERTIES UNITY_BUILD ON)
endif()

generate_export_header(KF5KontactInterface BASE_NAME kontactinterface)

target_include_directories(KF5KontactInterface INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KontactInterface>")
target_include_directories(KF5KontactInterface PUBLIC "$<BUILD_INTERFACE:${KontactInterface_SOURCE_DIR}/src;${KontactInterface_BINARY_DIR}/src;${KontactInterface_BINARY_DIR}>")

target_link_libraries(KF5KontactInterface
                     PUBLIC
                      KF5::CoreAddons
                      KF5::Parts
                      KF5::XmlGui
                     PRIVATE
                      KF5::WindowSystem
                      KF5::I18n
)

set_target_properties(KF5KontactInterface PROPERTIES
    VERSION ${KONTACTINTERFACE_VERSION}
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
  Processes
  PREFIX KontactInterface
  REQUIRED_HEADERS KontactInterface_HEADERS
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kontactinterface_export.h
  ${KontactInterface_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KontactInterface/kontactinterface
  COMPONENT Devel
)

install(FILES
    ${KontactInterface_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KontactInterface/KontactInterface/
    COMPONENT Devel
)


install(FILES kontactplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})

ecm_generate_pri_file(BASE_NAME KontactInterface LIB_NAME KF5KontactInterface DEPS "KParts KCoreAddons KXmlGui" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KontactInterface)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

ecm_qt_install_logging_categories(EXPORT KONTACTINTERFACE FILE kontactinterface.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
