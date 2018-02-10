add_definitions(-DENUT_COMPILING)

set(HEADERS
	datagramsocket.h
	enetsocket.h
	ipv4address.h
	ipv6address.h
	multicasttunnelsocket.h
	networklayeraddress.h
	nuttypes_win.h
	packet.h
	socket.h
	socketwaiter.h
	tcppacketsocket.h
	tcpsocket.h
	tcpv4socket.h
	tcpv6socket.h
	udpsocket.h
	udpv4socket.h
	udpv6socket.h
	${PROJECT_BINARY_DIR}/src/nutconfig.h
	${PROJECT_BINARY_DIR}/src/nuttypes.h )

set(SOURCES
	enetsocket.cpp
	ipv4address.cpp
	ipv6address.cpp
	multicasttunnelsocket.cpp
	socket.cpp
	socketwaiter.cpp
	tcppacketsocket.cpp
	tcpv4socket.cpp
	tcpv6socket.cpp
	udpv4socket.cpp
	udpv6socket.cpp )

if (MSVC)
	set(CMAKE_DEBUG_POSTFIX _d)
endif (MSVC)

if (NOT MSVC OR ENUT_COMPILE_STATIC)
	add_library(enut-static STATIC ${SOURCES} ${HEADERS})
	set_target_properties(enut-static PROPERTIES OUTPUT_NAME enut)
	set_target_properties(enut-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set(ENUT_INSTALLTARGETS enut-static)
	target_link_libraries(enut-static ${ENUT_LINK_LIBS})
endif (NOT MSVC OR ENUT_COMPILE_STATIC)

if (NOT MSVC OR NOT ENUT_COMPILE_STATIC)
	add_library(enut-shared SHARED ${SOURCES} ${HEADERS})
	set_target_properties(enut-shared PROPERTIES OUTPUT_NAME enut)
	set_target_properties(enut-shared PROPERTIES VERSION ${VERSION})
	set_target_properties(enut-shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set(ENUT_INSTALLTARGETS ${ENUT_INSTALLTARGETS} enut-shared)
	target_link_libraries(enut-shared ${ENUT_LINK_LIBS})
endif (NOT MSVC OR NOT ENUT_COMPILE_STATIC)

apply_include_paths("${ENUT_INTERNAL_INCLUDES}")
apply_include_paths("${ENUT_EXTERNAL_INCLUDES}")

install(FILES ${HEADERS} DESTINATION include/enut)
install(TARGETS ${ENUT_INSTALLTARGETS} DESTINATION ${LIBRARY_INSTALL_DIR})


