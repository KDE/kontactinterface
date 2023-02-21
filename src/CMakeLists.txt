# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

add_library(KPim${KF_MAJOR_VERSION}KontactInterface)
add_library(KPim${KF_MAJOR_VERSION}::KontactInterface ALIAS KPim${KF_MAJOR_VERSION}KontactInterface)
add_library(KF5::KontactInterface ALIAS KPim${KF_MAJOR_VERSION}KontactInterface)

target_sources(KPim${KF_MAJOR_VERSION}KontactInterface PRIVATE
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

ecm_qt_declare_logging_category(KPim${KF_MAJOR_VERSION}KontactInterface HEADER kontactinterface_debug.h IDENTIFIER KONTACTINTERFACE_LOG CATEGORY_NAME org.kde.pim.kontactinterface
        OLD_CATEGORY_NAMES log_kontactinterface
        DESCRIPTION "kontactinterface (pim lib)" EXPORT KONTACTINTERFACE)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim${KF_MAJOR_VERSION}KontactInterface PROPERTIES UNITY_BUILD ON)
endif()

generate_export_header(KPim${KF_MAJOR_VERSION}KontactInterface BASE_NAME kontactinterface)

target_include_directories(KPim${KF_MAJOR_VERSION}KontactInterface INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KontactInterface>")
target_include_directories(KPim${KF_MAJOR_VERSION}KontactInterface PUBLIC "$<BUILD_INTERFACE:${KontactInterface_SOURCE_DIR}/src;${KontactInterface_BINARY_DIR}/src;${KontactInterface_BINARY_DIR}>")

target_link_libraries(KPim${KF_MAJOR_VERSION}KontactInterface
                     PUBLIC
                      KF${KF_MAJOR_VERSION}::CoreAddons
                      KF${KF_MAJOR_VERSION}::Parts
                      KF${KF_MAJOR_VERSION}::XmlGui
                     PRIVATE
                      KF${KF_MAJOR_VERSION}::WindowSystem
                      KF${KF_MAJOR_VERSION}::I18n
                      KF${KF_MAJOR_VERSION}::KIOGui
)

if (KONTACTINTERFACE_HAVE_X11)
    if (QT_MAJOR_VERSION STREQUAL "5")
        target_link_libraries(KPim${KF_MAJOR_VERSION}KontactInterface PRIVATE Qt::X11Extras)
    else()
        target_link_libraries(KPim${KF_MAJOR_VERSION}KontactInterface PRIVATE Qt::GuiPrivate)
    endif()
endif()

set_target_properties(KPim${KF_MAJOR_VERSION}KontactInterface PROPERTIES
    VERSION ${KONTACTINTERFACE_VERSION}
    SOVERSION ${KONTACTINTERFACE_SOVERSION}
    EXPORT_NAME KontactInterface
)


install(TARGETS KPim${KF_MAJOR_VERSION}KontactInterface EXPORT KPim${KF_MAJOR_VERSION}KontactInterfaceTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

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
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KontactInterface/kontactinterface
  COMPONENT Devel
)

install(FILES
    ${KontactInterface_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KontactInterface/KontactInterface/
    COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME KontactInterface LIB_NAME KPim${KF_MAJOR_VERSION}KontactInterface DEPS "KParts KCoreAddons KXmlGui"
    FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KontactInterface)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

ecm_qt_install_logging_categories(EXPORT KONTACTINTERFACE FILE kontactinterface.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

if (BUILD_QCH)
    ecm_add_qch(
        KPim${KF_MAJOR_VERSION}KontactInterface_QCH
        NAME KPim${KF_MAJOR_VERSION}KontactInterface
        BASE_NAME KPim${KF_MAJOR_VERSION}KontactInterface
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
             ${KontactInterface_HEADERS}
        #MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        #IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt${QT_MAJOR_VERSION}Core_QCH
            Qt${QT_MAJOR_VERSION}Gui_QCH
            Qt${QT_MAJOR_VERSION}Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KONTACTINTERFACE_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
