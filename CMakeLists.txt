cmake_minimum_required(VERSION 3.13)

project (urnet VERSION 0.6 )
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-function -g  ")
elseif(UNIX)
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread -g -O2  -fPIC -Wno-unused-function ")
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread -g   -Wno-unused-function -fvisibility=hidden  ")
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread -g -static-libgcc  -static-libstdc++  -Wl,-Bstatic -lc -Wl,-Bdynamic   -Wno-unused-function -fvisibility=hidden  ")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread -g  -Wno-unused-function -fvisibility=hidden  -fPIC ")
 
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread -g -static-libgcc  -static-libstdc++  -Wl,-Bstatic -lc -Wl,-Bdynamic  -Wl,-rpath,bin   -Wno-unused-function -fvisibility=hidden  ")
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath ${CMAKE_CURRENT_SOURCE_DIR}/libs/ssl/lib/ ")
	link_directories(libs/ssl/lib)
elseif(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D NDEBUG")
endif()


include_directories(.)
include_directories(mods)
include_directories(include)
include_directories(utils)
include_directories(libs/)




add_subdirectory(opt/fmt-7.1.3  EXCLUDE_FROM_ALL)
add_subdirectory(opt/spdlog  EXCLUDE_FROM_ALL)
include_directories(opt/fmt-7.1.3/include)
include_directories(opt/spdlog/include)


 
 




file(GLOB SRCS 
	src/server.cpp
	)
add_executable(server ${SRCS} ) 
target_link_libraries(server uring  fmt)



add_executable(web src/web.cpp) 
target_link_libraries(web uring  fmt  )
