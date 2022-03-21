# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(PIM_VERSION "5.20.40")

project(KontactInterface VERSION ${PIM_VERSION})

# ECM setup
set(KF5_MIN_VERSION "5.92.0")

find_package(ECM ${KF5_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)

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
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5KontactInterface")

ecm_setup_version(PROJECT VARIABLE_PREFIX KONTACTINTERFACE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kontactinterface_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KontactInterfaceConfigVersion.cmake"
                        SOVERSION 5
)

ecm_setup_qtplugin_macro_names(
    JSON_ARG2
        EXPORT_KONTACT_PLUGIN_WITH_JSON
    CONFIG_CODE_VARIABLE
        PACKAGE_SETUP_AUTOMOC_VARIABLES
)

########### Find packages ###########
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    Parts
    WindowSystem
    I18n
    XmlGui
)

if (NOT APPLE)
    find_package(X11)
endif()

set(KONTACTINTERFACE_HAVE_X11 ${X11_FOUND})
configure_file(config-kontactinterface.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kontactinterface.h )


configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5KontactInterfaceConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KontactInterfaceConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

########### Install Files ###########
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KontactInterfaceConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KontactInterfaceConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)


install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kontactinterface_version.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KontactInterface
   COMPONENT Devel
)


install(EXPORT KF5KontactInterfaceTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5KontactInterfaceTargets.cmake NAMESPACE KF5::)

ecm_set_disabled_deprecation_versions(QT 5.15.2
    KF 5.92.0
    KPARTS 5.76 # componentData()
)


option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
endif()
add_subdirectory(src)


kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)


if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5KontactInterface_QCH
        FILE KF5KontactInterfaceQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5KontactInterfaceQchTargets.cmake\")")
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

