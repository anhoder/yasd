cmake_minimum_required(VERSION 3.23)
project(yasd)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(include)
include_directories(thirdparty/tinyxml2)
include_directories(/opt/homebrew/opt/php@8.0/include/php)
include_directories(/opt/homebrew/opt/php@8.0/include/php/Zend)
include_directories(/opt/homebrew/opt/boost/include)

add_executable(yasd
        include/base.h
        include/base64.h
        include/buffer.h
        include/cmder_debugger.h
        include/common.h
        include/context.h
        include/dbgp.h
        include/debuger_mode_base.h
        include/global.h
        include/logger.h
        include/redirect_file_to_cin.h
        include/remote_debugger.h
        include/source_reader.h
        include/util.h
        include/watch_point.h
        include/zend_property_info.h
        src/base.cc
        src/base64.cc
        src/buffer.cc
        src/cmder_debugger.cc
        src/common.cc
        src/context.cc
        src/dbgp.cc
        src/debuger_mode_base.cc
        src/global.cc
        src/logger.cc
        src/remote_debugger.cc
        src/source_reader.cc
        src/util.cc
        thirdparty/tinyxml2/tinyxml2.cc
        thirdparty/tinyxml2/tinyxml2.h
        php_yasd.h
        php_yasd_cxx.cc
        php_yasd_cxx.h
        yasd.cc
        yasd_api.cc
        yasd_api.h
        yasd_function_status.cc
        yasd_function_status.h)
