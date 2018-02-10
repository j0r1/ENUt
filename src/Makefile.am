lib_LTLIBRARIES = libnetutil.la
bin_PROGRAMS = multicasttunnelserver
libnetutil_la_SOURCES = \
tcpv4socket.cpp \
tcpv6socket.cpp \
udpv4socket.cpp \
udpv6socket.cpp \
socketwaiter.cpp \
enetsocket.cpp \
multicasttunnelsocket.cpp
libnetutilinclude_HEADERS = \
tcpv4socket.h \
tcpv6socket.h \
tcpsocket.h \
udpv4socket.h \
udpv6socket.h \
udpsocket.h \
nutconfig_unix.h \
nutconfig.h \
nutconfig_win.h \
nuttypes.h \
nuttypes_unix.h \
nuttypes_win.h \
errorbase.h \
ipv4address.h \
ipv6address.h \
networklayeraddress.h \
socket.h \
socketwaiter.h \
enetsocket.h \
multicasttunnelsocket.h \
datagramsocket.h
multicasttunnelserver_SOURCES = multicasttunnelserver.cpp
multicasttunnelserver_LDFLAGS = .libs/libnetutil.a -lenet
INCLUDES = 
libnetutil_la_LDFLAGS = -release @VERSION@
libnetutilincludedir = ${includedir}/netutil

