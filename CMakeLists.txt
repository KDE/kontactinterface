cmake_minimum_required(VERSION 2.8.12)

project(KontactInterface)

# ECM setup
find_package(ECM 5.19.0 CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)


add_definitions(-DTRANSLATION_DOMAIN=\"kontactinterfaces5\")
set(KF5_VERSION "5.19.0")
set(KONTACTINTERFACE_LIB_VERSION "5.2.3")
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5KontactInterface")

ecm_setup_version(${KONTACTINTERFACE_LIB_VERSION} VARIABLE_PREFIX KONTACTINTERFACE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kontactinterface_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KontactInterfaceConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########
find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS
    CoreAddons
    Parts
    WindowSystem
    I18n
    XmlGui
    IconThemes
)

if (NOT APPLE)
    find_package(X11)
endif()

set(KONTACTINTERFACE_HAVE_X11 ${X11_FOUND})
configure_file(config-kontactinterface.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kontactinterface.h )


ecm_configure_package_config_file(
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

add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")

add_subdirectory(src)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

