# ÌíŒÓµÚÈý·œÒÀÀµ°ü
include(FetchContent)
# FetchContent_MakeAvailable was not added until CMake 3.14
if(${CMAKE_VERSION} VERSION_LESS 3.18)
    include(to_work_low_version.cmake)
endif()

# set fmt's url
set(fmt_url  https://gitee.com/mohistH/fmt.git)  
# set fmt git's version
set(fmt_ver  master) 
# cmakelists.txt/ext in the top directory 
set(fmt_dir ${CMAKE_CURRENT_SOURCE_DIR}/ext/fmt) 

FetchContent_Declare(
  fmt
  GIT_REPOSITORY    ${fmt_url}
  GIT_TAG           ${fmt_ver}
  SOURCE_DIR		${fmt_dir}
)

FetchContent_MakeAvailable(fmt)
