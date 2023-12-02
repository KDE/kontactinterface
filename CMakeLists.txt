# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(PIM_VERSION "5.240.85")

project(KontactInterface VERSION ${PIM_VERSION})

# ECM setup
set(KF_MIN_VERSION "5.246.0")
set(QT_REQUIRED_VERSION "6.5.0")
find_package(ECM ${KF_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateExportHeader)
include(ECMGenerateHeaders)

include(ECMSetupVersion)
include(FeatureSummary)
include(KDEGitCommitHooks)
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMSetupQtPluginMacroNames)
include(ECMAddQch)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

add_definitions(-DTRANSLATION_DOMAIN=\"kontactinterfaces5\")

set(KONTACTINTERFACE_LIB_VERSION ${PIM_VERSION})
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KPim6KontactInterface")

ecm_setup_version(PROJECT VARIABLE_PREFIX KONTACTINTERFACE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/kontactinterface_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPim6KontactInterfaceConfigVersion.cmake"
                        SOVERSION 6
)

ecm_setup_qtplugin_macro_names(
    JSON_ARG2
        EXPORT_KONTACT_PLUGIN_WITH_JSON
    CONFIG_CODE_VARIABLE
        PACKAGE_SETUP_AUTOMOC_VARIABLES
)

########### Find packages ###########
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    Parts
    WindowSystem
    I18n
    XmlGui
)

if (NOT APPLE AND NOT WIN32)
    find_package(X11 REQUIRED)
endif()

option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
endif()
add_definitions(-DQT_NO_CONTEXTLESS_CONNECT)
ecm_set_disabled_deprecation_versions(QT 6.6
     KF 5.245.0
    KPARTS 5.76 # componentData()
)
set(KONTACTINTERFACE_HAVE_X11 ${X11_FOUND})

add_subdirectory(src)

configure_file(config-kontactinterface.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-kontactinterface.h )
if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KPim6KontactInterface_QCH
        FILE KPim6KontactInterfaceQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KPim6KontactInterfaceQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KPimKontactInterfaceConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KPim6KontactInterfaceConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

########### Install Files ###########
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KPim6KontactInterfaceConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KPim6KontactInterfaceConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)


install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/src/kontactinterface_version.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KontactInterface
   COMPONENT Devel
)


install(EXPORT KPim6KontactInterfaceTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KPim6KontactInterfaceTargets.cmake NAMESPACE KPim6::)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

