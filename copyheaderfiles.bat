@ECHO OFF
IF EXIST %1\include (
        echo Include directory already exists
) ELSE (
        echo Creating include directory
        md %1\include
)

IF EXIST %1\include\enut (
	echo Include subdirectory already exists
) ELSE (
	echo Creating include subdirectory
	md %1\include\enut
)

copy %1\src\datagramsocket.h %1\include\enut\
copy %1\src\enetsocket.h %1\include\enut\
copy %1\src\ipv4address.h %1\include\enut\
copy %1\src\ipv6address.h %1\include\enut\
copy %1\src\multicasttunnelserver.h %1\include\enut\
copy %1\src\multicasttunnelsocket.h %1\include\enut\
copy %1\src\networklayeraddress.h %1\include\enut\
copy %1\src\nutconfig.h %1\include\enut\
copy %1\src\nutconfig_unix.h %1\include\enut\
copy %1\src\nutconfig_win.h %1\include\enut\
copy %1\src\nuttypes.h %1\include\enut\
copy %1\src\nuttypes_unix.h %1\include\enut\
copy %1\src\nuttypes_win.h %1\include\enut\
copy %1\src\packet.h %1\include\enut\
copy %1\src\socket.h %1\include\enut\
copy %1\src\socketwaiter.h %1\include\enut\
copy %1\src\tcppacketsocket.h %1\include\enut\
copy %1\src\tcpsocket.h %1\include\enut\
copy %1\src\tcpv4socket.h %1\include\enut\
copy %1\src\tcpv6socket.h %1\include\enut\
copy %1\src\udpsocket.h %1\include\enut\
copy %1\src\udpv4socket.h %1\include\enut\
copy %1\src\udpv6socket.h %1\include\enut\
