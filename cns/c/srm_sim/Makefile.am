# what flags you want to pass to the C compiler & linker
AM_CFLAGS = -Wall -std=c99 -O0 -g
#CFLAGS=
LDADD= -lactor -lm

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
#bin_PROGRAMS = srm_sim srm_sim_ut
#bin_PROGRAMS = $(top_builddir)/bin/srm_sim $(top_builddir)/bin/srm_sim_ut
bin_PROGRAMS = $(top_builddir)/bin/srm_sim
__top_builddir__bin_srm_sim_SOURCES = ./srm_sim.c ./layer.c  ./matrix.c  ./util/util_vector.c ./util/ini.c ./templates.h ./layer.h ./matrix.h ./core.h ./util/util_vector.h ./util/ini.h ./sim.h constants.c util/util.c sim.c io.c io.h spikes_list.c spikes_list.h net_sim.c net_sim.h neuron_funcs.c
__top_builddir__bin_srm_sim_ut_SOURCES = ./srm_sim_ut.c ./layer.c  ./matrix.c  ./util/util_vector.c ./util/ini.c ./templates.h ./layer.h ./matrix.h ./core.h ./util/util_vector.h ./util/ini.h ./sim.h constants.c util/util.c sim.c io.c io.h spikes_list.c spikes_list.h net_sim.c net_sim.h
