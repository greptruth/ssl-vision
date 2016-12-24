INCLUDE(FindPackageHandleStandardArgs)

FIND_LIBRARY(BASLER_PYLON_LIBRARY pylonbase "$ENV{BASLER_PYLON_LIB_PATH}"
  /opt/pylon4/lib
  /opt/pylon4/lib32
  /opt/pylon4/lib64
  )

FIND_PATH(BASLER_PYLON_INCLUDE_PATH pylon/Device.h
  "$ENV{BASLER_PYLON_INCLUDE_PATH}"
  /opt/pylon4/include
)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BASLER_PYLON DEFAULT_MSG
                                             BASLER_PYLON_INCLUDE_PATH
                                             BASLER_PYLON_LIBRARY)
