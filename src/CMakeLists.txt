add_compile_definitions(NOMINMAX)
add_compile_definitions(SKSE_SUPPORT_XBYAK)

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

set(SOURCE_DIR "${ROOT_DIR}/src")
set(SOURCE_FILES
	"${SOURCE_DIR}/Animation/Animation.h"
	"${SOURCE_DIR}/CLIK/GFx/Controls/Button.h"
	"${SOURCE_DIR}/CLIK/GFx/Controls/ButtonBar.h"
	"${SOURCE_DIR}/CLIK/GFx/Controls/CoreList.h"
	"${SOURCE_DIR}/CLIK/GFx/Controls/DropdownMenu.h"
	"${SOURCE_DIR}/CLIK/GFx/Controls/ScrollingList.h"
	"${SOURCE_DIR}/CLIK/GFx/Controls/Slider.h"
	"${SOURCE_DIR}/CLIK/GFx/Controls/TextInput.h"
	"${SOURCE_DIR}/CLIK/GFx/Core/UIComponent.h"
	"${SOURCE_DIR}/CLIK/Array.h"
	"${SOURCE_DIR}/CLIK/MovieClip.h"
	"${SOURCE_DIR}/CLIK/Object.h"
	"${SOURCE_DIR}/CLIK/TextField.h"
	"${SOURCE_DIR}/Events/Events.cpp"
	"${SOURCE_DIR}/Events/Events.h"
	"${SOURCE_DIR}/Input/Input.h"
	"${SOURCE_DIR}/Input/InputDisablers.h"
	"${SOURCE_DIR}/Input/InputListeners.cpp"
	"${SOURCE_DIR}/Input/InputListeners.h"
	"${SOURCE_DIR}/Items/GFxItem.cpp"
	"${SOURCE_DIR}/Items/GFxItem.h"
	"${SOURCE_DIR}/Items/GFxItemCache.hpp"
	"${SOURCE_DIR}/Items/GroundItem.h"
	"${SOURCE_DIR}/Items/InventoryItem.h"
	"${SOURCE_DIR}/Items/Item.h"
	"${SOURCE_DIR}/Scaleform/LootMenu.cpp"
	"${SOURCE_DIR}/Scaleform/LootMenu.h"
	"${SOURCE_DIR}/Scaleform/Scaleform.cpp"
	"${SOURCE_DIR}/Scaleform/Scaleform.h"
	"${SOURCE_DIR}/ContainerChangedHandler.cpp"
	"${SOURCE_DIR}/ContainerChangedHandler.h"
	"${SOURCE_DIR}/Hooks.cpp"
	"${SOURCE_DIR}/Hooks.h"
	"${SOURCE_DIR}/HUDManager.h"
	"${SOURCE_DIR}/Loot.cpp"
	"${SOURCE_DIR}/Loot.h"
	"${SOURCE_DIR}/main.cpp"
	"${SOURCE_DIR}/OpenCloseHandler.h"
	"${SOURCE_DIR}/PCH.h"
	"${SOURCE_DIR}/Settings.cpp"
	"${SOURCE_DIR}/Settings.h"
	"${SOURCE_DIR}/ViewHandler.cpp"
	"${SOURCE_DIR}/ViewHandler.h"
	"${SOURCE_DIR}/LOTD/LOTD.cpp"
	"${SOURCE_DIR}/LOTD/LOTD.h"
)

source_group(TREE "${ROOT_DIR}" FILES ${SOURCE_FILES})

set(VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/include/Plugin.h")
configure_file(
	"${ROOT_DIR}/cmake/Plugin.h.in"
	"${VERSION_HEADER}"
	@ONLY
)

source_group("include" FILES "${VERSION_HEADER}")

configure_file(
	"${ROOT_DIR}/cmake/version.rc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/version.rc"
	@ONLY
)

find_package(frozen REQUIRED CONFIG)
find_package(spdlog REQUIRED CONFIG)
find_package(srell MODULE REQUIRED)
find_package(directxtk CONFIG REQUIRED)
find_package(CommonLibSSE CONFIG REQUIRED)
find_package(xbyak CONFIG REQUIRED)

add_commonlibsse_plugin(
	"${PROJECT_NAME}"
	SOURCES
	${SOURCE_FILES}
	"${VERSION_HEADER}"
	"${CMAKE_CURRENT_BINARY_DIR}/version.rc"
	"${ROOT_DIR}/.clang-format"
	"${ROOT_DIR}/.editorconfig"
)

target_compile_features(
	"${PROJECT_NAME}"
	PRIVATE
	cxx_std_20
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	target_compile_options(
		"${PROJECT_NAME}"
		PRIVATE
		"/sdl" # Enable Additional Security Checks
		"/utf-8" # Set Source and Executable character sets to UTF-8
		"/Zi" # Debug Information Format

		"/permissive-" # Standards conformance
		"/Zc:preprocessor" # Enable preprocessor conformance mode

		"/wd4200" # nonstandard extension used : zero-sized array in struct/union

		"$<$<CONFIG:DEBUG>:>"
		"$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"
	)

	target_link_options(
		"${PROJECT_NAME}"
		PRIVATE
		"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
		"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
	)
endif()

target_include_directories(
	"${PROJECT_NAME}"
	PRIVATE
	"${CMAKE_CURRENT_BINARY_DIR}/include"
	"${INCLUDE_DIR}"
	"${SOURCE_DIR}"
)

target_link_libraries(
	"${PROJECT_NAME}"
	PRIVATE
	CommonLibSSE::CommonLibSSE
	frozen::frozen
	spdlog::spdlog
	srell::srell
	xbyak::xbyak
)

target_precompile_headers(
	"${PROJECT_NAME}"
	PRIVATE
	"${SOURCE_DIR}/PCH.h"
)

if("${COPY_OUTPUT}")
	add_custom_command(
		TARGET "${PROJECT_NAME}"
		POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${Skyrim64Path}/Data/SKSE/Plugins/"
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${Skyrim64Path}/Data/SKSE/Plugins/"
		VERBATIM
	)
endif()
