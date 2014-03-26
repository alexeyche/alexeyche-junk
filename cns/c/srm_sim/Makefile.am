# what flags you want to pass to the C compiler & linker
AM_CFLAGS = -Wall -std=c99 -O0 -g
LDADD= -lactor

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = srm_sim srm_sim_ut
bin_PROGRAMS = $(top_builddir)/bin/srm_sim
__top_builddir__bin_srm_sim_SOURCES = ./srm_sim.c ./layer.c  ./matrix.c  ./util/util_vector.c ./util/ini.c ./templates.h ./layer.h ./matrix.h ./core.h ./util/util_vector.h ./util/ini.h ./sim.h constants.c util/util.c sim.c io.c io.h
__top_builddir__bin_srm_sim_ut_SOURCES = ./layer.c ./srm_sim_ut.c ./matrix.c  ./util/util_vector.c ./util/ini.c ./templates.h ./layer.h ./matrix.h ./core.h ./util/util_vector.h ./util/ini.h ./sim.h ./constants.c util/util.c sim.c io.c io.h

