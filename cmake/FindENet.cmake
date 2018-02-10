
find_path(ENET_INCLUDE_DIR enet/enet.h)

set(ENET_INCLUDE_DIRS ${ENET_INCLUDE_DIR})

find_library(ENET_LIBRARY enet)
if (ENET_LIBRARY)
	set(ENET_LIBRARIES ${ENET_LIBRARY})
endif (ENET_LIBRARY)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(ENet DEFAULT_MSG ENET_INCLUDE_DIRS ENET_LIBRARIES)

