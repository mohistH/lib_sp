cmake_minimum_required(VERSION 3.18)

#工程名
project(lib_serial_port VERSION 1.0.1)


# library version
# -------------------------------------------------------------------------------------
string(TIMESTAMP str_ver_date "%d%m%Y")
set(version_1 ${PROJECT_VERSION_MAJOR})
set(version_2 ${PROJECT_VERSION_MINOR})
set(version_3 ${PROJECT_VERSION_PATCH})
set(version_4 ${str_ver_date}) 

# set version information
# -------------------------------------------------------------------------------------
set (FileDescription 		"A library to operate serial port")
set (InternalName 			"Serial Port")
set (copyright_from 		"mohistH")
set (OriginalFilename 		"serial_port(64).dll")
set (ProductName			"Serial Port Library")




# PRIVATE	配置仅在编译的时候需要满足
# INTERFACE 类型的配置则是在使用时需要满足，即在其他项目里，
#    		使用本项目已编译好的 target 时需要满足
# PUBLIC	实际工程中，有很多配置在编译时以及被使用时都需要被满足
# -------------------------------------------------------------------------------------
option(example_shared 		"an example to show how to use dynamic library" ON)
option(lib_sp_is_shared 	"ON=SHARED library, OFF=STATIC library" 		ON)
option(used_in_mfc 			"Create a library used in MFC? " 				OFF)
option(lib_sp_is_x64		"Generate x64 library ?"						OFF)


# to copy files
# -------------------------------------------------------------------------------------
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/config/config.h.in
	${CMAKE_CURRENT_SOURCE_DIR}/include/config.h
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/config/version.rc.in
	${CMAKE_CURRENT_SOURCE_DIR}/src/version.rc
)


# to set library type
# -------------------------------------------------------------------------------------
if(lib_sp_is_shared)
	set(lib_type SHARED)
else()
	set(lib_type STATIC)
endif()

# check if need to build x64
# -------------------------------------------------------------------------------------
if(CMAKE_CL_64)
	set(build_64_lib_sp ON)
else()
	if (lib_sp_is_x64)
		set(CMAKE_CL_64 ON)
		set(build_64_lib_sp ON)
	endif(lib_sp_is_x64)
endif(CMAKE_CL_64)

# set_project_name
# -------------------------------------------------------------------------------------
set(lib_name "")

# set header files
# -------------------------------------------------------------------------------------
file(GLOB_RECURSE sp_inc ${CMAKE_CURRENT_SOURCE_DIR}/include/*h)

# set source files
# -------------------------------------------------------------------------------------
file(GLOB_RECURSE sp_src ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)


#编译时的宏定义：
# target_compile_definitions(serial_port PRIVATE LogLevel=3)

#如果你有一些参数想直接传给底层的编译器（比如 gcc, clang, cl），
# target_compile_options(MyLib PRIVATE -Werror -Wall -Wextra)

# 其他选项
# target_source(serial_port PRIVATE ${sp_src} ${sp_inc})
# target_link_library(MyEXE PRIVATE Poco::Net Poco::Util)
# target_compile_definition(MyEXE PRIVATE std_cxx_14)


function (lib_sp_build_win used_in_mfc_flag )
	# windows
	IF (CMAKE_SYSTEM_NAME MATCHES "Windows")

		if(used_in_mfc_flag)
			set(CMAKE_MFC_FLAG 2)
			set(CMAKE_CXX_FLAGS_RELEASE "/MT")
     		set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
		endif(used_in_mfc_flag)
		
		# create serial_port project
		# add_library(serial_port ${lib_type} )
				
		if(MSVC)
			set(version_rc_win "${CMAKE_CURRENT_SOURCE_DIR}/src/version.rc")
			add_library(${lib_name} ${lib_type} ${version_rc_win})
		else()
			add_library(${lib_name} ${lib_type} )
		endif(MSVC)
			
		if(used_in_mfc_flag)
			# target_compile_definitions(serial_port PRIVATE _AFXDLL)
			target_compile_definitions(${lib_name} PRIVATE _AFXDLL)
			
			# shared dll
			if(lib_type)
				set(CMAKE_MFC_FLAG  2)
			# static dll
			else()
				set(CMAKE_MFC_FLAG  1)
			endif()
		endif(used_in_mfc_flag)

		# .h and .cxx files
		target_sources(${lib_name} PRIVATE ${sp_src} ${sp_inc})

		if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
			# set vs support cxx11
			if (MSVC_VERSION GREATER_EQUAL "1910")	# 1910 = VS2017 15.0
				add_compile_options("/Zc:__cplusplus")
				target_compile_definitions(${lib_name} PRIVATE _X86_ UNICODE _UNICODE)				
			endif()
		endif()	
	
		
		# 
		target_link_libraries(${lib_name} PRIVATE setupapi)
		# set 
		target_include_directories(${lib_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/)
		# use c++11
		target_compile_features(${lib_name} PRIVATE cxx_std_11)
		# vesion
		set(version_str ${version_1}.${version_2}.${version_3}.${version_4})
		message("version=${version_str}")
		# set(ver_str "")
		# string(REPLACE ";" "." ${ver_str} ${version_str})
		# message("version=${version_str}")
		set_target_properties(${lib_name} PROPERTIES VERSION ${version_str} SOVERSION 1)
		
	ENDIF (CMAKE_SYSTEM_NAME MATCHES "Windows")

endfunction()



# used this library in mfc ? 
#-------------------------------------------------------------------------------------
# if(used_in_mfc)
	# set(CMAKE_CXX_FLAGS_RELEASE "/MT")
    # set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
# endif(used_in_mfc)

# create shared or static library project
# -------------------------------------------------------------------------------------
if(lib_sp_is_x64)
	set(lib_name "serial_port64")
else()
	set(lib_name "serial_port")
endif(lib_sp_is_x64)

lib_sp_build_win(used_in_mfc)


# create an example of shared library
# -------------------------------------------------------------------------------------
if(example_shared)

	# set header files
	file(GLOB_RECURSE exam_file_h 
		${CMAKE_CURRENT_SOURCE_DIR}/include/universe_def.h 
		${CMAKE_CURRENT_SOURCE_DIR}/include/serial_port/serial_port_interface.h
	)

	# set source files
	file(GLOB_RECURSE exam_file_cc 
		${CMAKE_CURRENT_SOURCE_DIR}/example/main.cc 
	)

	# create an object
	add_executable(example_shared)
	# .h and .cxx files
	target_sources(example_shared PRIVATE ${exam_file_h} ${exam_file_cc})
	#
	target_link_libraries(example_shared PRIVATE ${lib_name})
	#  
	target_include_directories(example_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
	#
	target_compile_definitions(example_shared PRIVATE  _UNICODE)

endif(example_shared)




# 判断操作系统
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	
ELSE ()
	MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")


## 判断编译器
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
# using Clang
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
# using GCC
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
# using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
# using Visual Studio C++
endif()


