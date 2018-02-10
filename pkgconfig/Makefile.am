### all of the standard pc files we need to generate
pcfiles = 							\
	enut.pc

pcfiles_uninstalled =						\
	enut-uninstalled.pc

all-local: $(pcfiles) $(pcfiles_uninstalled)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pcfiles)

EXTRA_DIST =							\
	enut.pc.in						\
	enut-uninstalled.pc.in

CLEANFILES = $(pcfiles) $(pcfiles_uninstalled)
