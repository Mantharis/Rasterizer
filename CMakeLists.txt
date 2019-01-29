cmake_minimum_required (VERSION 3.10)

set(CMAKE_SUPPRESS_REGENERATION true)
set_property(GLOBAL PROPERTY USE_FOLDERS ON) 

project (Wolverine)

file(GLOB WOLVERINE_SRC
    "${PROJECT_SOURCE_DIR}/*.h"
    "${PROJECT_SOURCE_DIR}/*.cpp"
)
source_group(WOLVERINE   FILES ${WOLVERINE_SRC})

file(GLOB SIMD_SRC
	"${PROJECT_SOURCE_DIR}/SIMD/Base/*.h"
	"${PROJECT_SOURCE_DIR}/SIMD/SSE/*.h"
)
source_group(SIMD   FILES ${SIMD_SRC})

set( ALL_SRC
	${WOLVERINE_SRC}
	${SIMD_SRC}
)
	
add_executable(Wolverine ${ALL_SRC})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Wolverine)

set_target_properties(Wolverine PROPERTIES LINKER_LANGUAGE CXX)

set(INCLUDE_DIRS 
	"c:\\SDK programming\\gmtl-0.6.1\\" 
	"c:\\SDK programming\\CImg-2.3.2\\" 
	"c:\\SDK programming\\tinyobjloader\\" 
	"c:\\SDK programming\\SDL2-2.0.8\\include\\"
	)
	
target_include_directories(Wolverine PUBLIC ${INCLUDE_DIRS})

set(DLL_PATHS 
	"c:\\SDK programming\\SDL2-2.0.8\\lib\\x64\\SDL2.lib" 
	"c:\\SDK programming\\SDL2-2.0.8\\lib\\x64\\SDL2main.lib")
	
target_link_libraries(Wolverine ${DLL_PATHS})