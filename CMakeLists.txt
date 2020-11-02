cmake_minimum_required(VERSION 3.16)

project(eomap4 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILTIN_DATA_SRCFILE "${CMAKE_CURRENT_BINARY_DIR}/generated/builtin_data.c")

# ---

set(CMAKE_FIND_LIBRARY_SUFFIXES
	"${CMAKE_STATIC_LIBRARY_SUFFIX}"
	"${CMAKE_SHARED_LIBRARY_SUFFIX}"
	"${CMAKE_SHARED_MODULE_SUFFIX}"
)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(PkgConfig REQUIRED)

pkg_check_modules(ALLEGRO5            REQUIRED IMPORTED_TARGET allegro-static-5)
pkg_check_modules(ALLEGRO5_FONT       REQUIRED IMPORTED_TARGET allegro_font-static-5)
pkg_check_modules(ALLEGRO5_IMAGE      REQUIRED IMPORTED_TARGET allegro_image-static-5)
pkg_check_modules(ALLEGRO5_PRIMITIVES REQUIRED IMPORTED_TARGET allegro_primitives-static-5)
pkg_check_modules(ALLEGRO5_DIALOG     REQUIRED IMPORTED_TARGET allegro_dialog-static-5)
pkg_check_modules(ALLEGRO5_PHYSFS     REQUIRED IMPORTED_TARGET allegro_physfs-static-5)
pkg_check_modules(PHYSFS              REQUIRED IMPORTED_TARGET physfs)

# ---

add_library(a5ses STATIC
	a5ses/include/a5ses/a5ses.hpp
	a5ses/include/a5ses/Atlas.hpp
	a5ses/include/a5ses/Bitmap.hpp
	a5ses/include/a5ses/Color.hpp
	a5ses/include/a5ses/common.hpp
	a5ses/include/a5ses/Display.hpp
	a5ses/include/a5ses/Event.hpp
	a5ses/include/a5ses/Event_Reader.hpp
	a5ses/include/a5ses/Keyboard.hpp
	a5ses/include/a5ses/Mouse.hpp
	a5ses/include/a5ses/Text.hpp
	a5ses/include/a5ses/Thread.hpp
	a5ses/include/a5ses/Timer.hpp
	a5ses/src/Atlas.cpp
)

target_link_libraries(a5ses PUBLIC
	PkgConfig::ALLEGRO5_STATIC
	PkgConfig::ALLEGRO5_FONT_STATIC
	PkgConfig::ALLEGRO5_IMAGE_STATIC
	PkgConfig::ALLEGRO5_PRIMITIVES_STATIC
	PkgConfig::ALLEGRO5_DIALOG_STATIC
	PkgConfig::ALLEGRO5_PHYSFS_STATIC
	PkgConfig::PHYSFS_STATIC
)

target_compile_definitions(a5ses PUBLIC A5SES_INT_COORD)

target_include_directories(a5ses PUBLIC a5ses/include)
target_include_directories(a5ses PRIVATE a5ses/include/a5ses)

add_executable(eomap4
	common.hpp
	cio.cpp
	cio.hpp
	crc32.c
	crc32.h
	dib_reader.cpp
	dib_reader.hpp
	eodata.cpp
	eodata.hpp
	EO_Map.cpp
	EO_Map.hpp
	GFX_Loader.cpp
	GFX_Loader.hpp
	GUI.cpp
	GUI.hpp
	main.cpp
	Map_Renderer.cpp
	Map_Renderer.hpp
	Palette.cpp
	Palette.hpp
	pe_reader.cpp
	pe_reader.hpp
	resource.h
	resource.rc
	util.cpp
	util.hpp

	${BUILTIN_DATA_SRCFILE}
)

target_compile_options(eomap4 PRIVATE -fwrapv)
target_link_libraries(eomap4 PRIVATE a5ses)

# -----

add_executable(bin2c
	bin2c.c
)

# -----

# https://stackoverflow.com/a/61385572
# doesn't work on initial configure, have to delete host_tools dir and re-configure
if(CMAKE_CROSSCOMPILING)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/host_tools)
	file(TO_NATIVE_PATH ${CMAKE_COMMAND} native_cmake_command)
	file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} native_cmake_current_source_dir)

	execute_process(
		COMMAND "${native_cmake_command}"
		        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
				"${native_cmake_current_source_dir}"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/host_tools"
	)

	add_custom_target(host_tools
		COMMAND "${CMAKE_COMMAND}" --build . --target host_tools --config "$<CONFIG>"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/host_tools"
	)

	include("${CMAKE_CURRENT_BINARY_DIR}/host_tools/host_tools.cmake")

	add_dependencies(host_bin2c host_tools)
else()
	add_custom_target(host_tools DEPENDS bin2c)
	add_executable(host_bin2c ALIAS bin2c)
	export(TARGETS bin2c NAMESPACE host_ FILE host_tools.cmake)
endif()

# -----

set(BUILTIN_DATA_SYMBOL builtin_data)
set(BUILTIN_DATA_ARCHIVE "${CMAKE_CURRENT_BINARY_DIR}/generated/builtin.zip")
set(BUILTIN_DATA_ARCHIVE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/data/")
set(BUILTIN_DATA_ARCHIVE_FILES
	.
	specs
)

set(BUILTIN_DATA_ARCHIVE_DEPS ${BUILTIN_DATA_ARCHIVE_FILES})
list(TRANSFORM BUILTIN_DATA_ARCHIVE_DEPS PREPEND "${BUILTIN_DATA_ARCHIVE_ROOT}/")

add_custom_command(
	OUTPUT "${BUILTIN_DATA_SRCFILE}"
	COMMAND zip -r "${BUILTIN_DATA_ARCHIVE}" .
	COMMAND
		"$<TARGET_FILE:host_bin2c>" "${BUILTIN_DATA_SYMBOL}"
		"${BUILTIN_DATA_ARCHIVE}" "${BUILTIN_DATA_SRCFILE}"
	WORKING_DIRECTORY "${BUILTIN_DATA_ARCHIVE_ROOT}"
	BYPRODUCTS "${BUILTIN_DATA_ARCHIVE}"
	DEPENDS ${BUILTIN_DATA_ARCHIVE_DEPS}
)
