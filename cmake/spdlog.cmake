# ÌíŒÓµÚÈý·œÒÀÀµ°ü
include(FetchContent)
# FetchContent_MakeAvailable was not added until CMake 3.14
if(${CMAKE_VERSION} VERSION_LESS 3.18)
    include(to_work_low_version.cmake)
endif()

# set spdlog's url
set(spdlog_url  https://gitee.com/mohistH/spdlog.git)  
# set sdplog git's version
set(spdlog_ver  v1.x) 
# top cmakelists.txt/ext	
set(spdlog_dir ${CMAKE_CURRENT_SOURCE_DIR}/ext/spdlog) 

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY    ${spdlog_url}
  GIT_TAG           ${spdlog_ver}
  SOURCE_DIR		${spdlog_dir}
)

FetchContent_MakeAvailable(spdlog)
