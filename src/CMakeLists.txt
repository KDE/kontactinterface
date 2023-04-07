# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

add_library(KPim6KontactInterface)
add_library(KPim6::KontactInterface ALIAS KPim6KontactInterface)

target_sources(KPim6KontactInterface PRIVATE
    core.cpp
    plugin.cpp
    summary.cpp
    processes.cpp
    uniqueapphandler.cpp
    pimuniqueapplication.cpp
    processes.h
    core.h
    plugin.h
    uniqueapphandler.h
    pimuniqueapplication.h
    summary.h)

ecm_qt_declare_logging_category(KPim6KontactInterface HEADER kontactinterface_debug.h IDENTIFIER KONTACTINTERFACE_LOG CATEGORY_NAME org.kde.pim.kontactinterface
        OLD_CATEGORY_NAMES log_kontactinterface
        DESCRIPTION "kontactinterface (pim lib)" EXPORT KONTACTINTERFACE)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim6KontactInterface PROPERTIES UNITY_BUILD ON)
endif()

ecm_generate_export_header(KPim6KontactInterface
    BASE_NAME kontactinterface
    VERSION ${KONTACTINTERFACE_VERSION}
    DEPRECATED_BASE_VERSION 0
    USE_VERSION_HEADER
)

target_include_directories(KPim6KontactInterface INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim6/KontactInterface>")
target_include_directories(KPim6KontactInterface PUBLIC "$<BUILD_INTERFACE:${KontactInterface_SOURCE_DIR}/src;${KontactInterface_BINARY_DIR}/src;${KontactInterface_BINARY_DIR}>")

target_link_libraries(KPim6KontactInterface
                     PUBLIC
                      KF6::CoreAddons
                      KF6::Parts
                      KF6::XmlGui
                     PRIVATE
                      KF6::WindowSystem
                      KF6::I18n
                      KF6::KIOGui
)

if (KONTACTINTERFACE_HAVE_X11)
    target_link_libraries(KPim6KontactInterface PRIVATE Qt::GuiPrivate)
endif()

set_target_properties(KPim6KontactInterface PROPERTIES
    VERSION ${KONTACTINTERFACE_VERSION}
    SOVERSION ${KONTACTINTERFACE_SOVERSION}
    EXPORT_NAME KontactInterface
)


install(TARGETS KPim6KontactInterface EXPORT KPim6KontactInterfaceTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

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
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KontactInterface/kontactinterface
  COMPONENT Devel
)

install(FILES
    ${KontactInterface_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KontactInterface/KontactInterface/
    COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME KontactInterface LIB_NAME KPim6KontactInterface DEPS "KParts KCoreAddons KXmlGui"
    FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/KPim6/KontactInterface)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

ecm_qt_install_logging_categories(EXPORT KONTACTINTERFACE FILE kontactinterface.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

if (BUILD_QCH)
    ecm_add_qch(
        KPim6KontactInterface_QCH
        NAME KPim6KontactInterface
        BASE_NAME KPim6KontactInterface
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
             ${KontactInterface_HEADERS}
        #MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        #IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt6Core_QCH
            Qt6Gui_QCH
            Qt6Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KONTACTINTERFACE_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
