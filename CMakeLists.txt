cmake_minimum_required(VERSION 2.8.12)

project(KontactInterface)

# ECM setup
find_package(ECM 0.0.14 CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)


add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5601)
add_definitions(-DTRANSLATION_DOMAIN=\"kontactinterfaces5\")
# FIXME: Do we want the same versions in kdepimlibs and kf5?
set(KF5_VERSION "4.98.0")
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5KontactInterface")

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KONTACTINTERFACE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kontactinterface_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KontactInterfaceConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########
find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Parts ${KF5_VERSION} CONFIG REQUIRED)

if(${CMAKE_SOURCE_DIR} STREQUAL ${KontactInterface_SOURCE_DIR})
  find_package(KF5PimUtils ${KF5_VERSION} CONFIG REQUIRED)
endif()


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
   DESTINATION ${KF5_INCLUDE_INSTALL_DIR}
   COMPONENT Devel
)


install(EXPORT KF5KontactInterfaceTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5KontactInterfaceTargets.cmake NAMESPACE KF5::)

add_subdirectory(src)
if ("${CMAKE_BINARY_DIR}" STREQUAL "${KontactInterface_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

