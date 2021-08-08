cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(PIM_VERSION "5.18.1")

project(KontactInterface VERSION ${PIM_VERSION})

# ECM setup
set(KF5_MIN_VERSION "5.83.0")

find_package(ECM ${KF5_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)

include(ECMSetupVersion)
include(FeatureSummary)
include(KDEGitCommitHooks)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupQtPluginMacroNames)



add_definitions(-DTRANSLATION_DOMAIN=\"kontactinterfaces5\")

set(KONTACTINTERFACE_LIB_VERSION ${PIM_VERSION})
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5KontactInterface")

ecm_setup_version(PROJECT VARIABLE_PREFIX KONTACTINTERFACE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kontactinterface_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KontactInterfaceConfigVersion.cmake"
                        SOVERSION 5
)

ecm_setup_qtplugin_macro_names(
    JSON_NONE
        EXPORT_KONTACT_PLUGIN
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
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
   DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}
   COMPONENT Devel
)


install(EXPORT KF5KontactInterfaceTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5KontactInterfaceTargets.cmake NAMESPACE KF5::)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054C00)


option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
endif()
add_subdirectory(src)


kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

