cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(app_flash_example)

set(APP_HW_TARGET          XK-EVK-XU316) #note this is the XCORE-AI-EXPLORER

set(APP_DEPENDENT_MODULES  "")

set(APP_COMPILER_FLAGS     -report
                            -lquadflash)

set(APP_INCLUDES           src/
                            src/uart/api
                            src/uart/src)

set(XMOS_SANDBOX_DIR       ${CMAKE_CURRENT_LIST_DIR}/../..)

XMOS_REGISTER_APP()
