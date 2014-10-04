# what flags you want to pass to the C compiler & linker
#AM_CFLAGS = -Wall -Wno-unused-variable -std=gnu99 -pthread  -O3 
AM_CFLAGS = -Wall -std=gnu99 -pthread -O0 -g  

MAIN_SOURCES = util/matrix.c util/util_vector.c ./util/ini.c ./util/matrix.h util/util.c util/spikes_list.c ./util/util_dlink_list.c sim/netsim.c neuron_funcs.c  constants.c util/io.c util/mem.c sim/runtime.c sim/configure.c sim/serialize.c ./layers/poisson.c sim/sim.c learn/triple_stdp.c learn/optim.c learn/res_stdp.c learn/simple_stdp.c layers/wta.c layers/adapt.c layers/wtaadapt.c

CFLAGS=
LDADD= -lm
# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = $(top_builddir)/bin/snn_sim $(top_builddir)/bin/snn_prep $(top_builddir)/bin/snn_calc $(top_builddir)/bin/snn_postproc  $(top_builddir)/bin/snn_filt
__top_builddir__bin_snn_sim_CFLAGS = $(AM_CFLAGS)
__top_builddir__bin_snn_sim_SOURCES = $(MAIN_SOURCES) snn_sim.c 

__top_builddir__bin_snn_prep_SOURCES = util/matrix.c util/util_vector.c ./util/ini.c ./util/matrix.h util/util.c util/spikes_list.c  constants.c util/io.c prep/adex.c  prep/run.c snn_prep.c 
__top_builddir__bin_snn_prep_CFLAGS = $(AM_CFLAGS)

__top_builddir__bin_snn_calc_SOURCES = util/matrix.c util/util_vector.c ./util/ini.c ./util/matrix.h util/util.c util/spikes_list.c  util/io.c snn_calc.c calc.c
__top_builddir__bin_snn_calc_CFLAGS = $(AM_CFLAGS)

__top_builddir__bin_snn_postproc_SOURCES = util/matrix.c util/util_vector.c ./util/ini.c ./util/matrix.h util/util.c util/spikes_list.c  util/io.c postproc.c snn_postproc.c
__top_builddir__bin_snn_postproc_CFLAGS = $(AM_CFLAGS)

__top_builddir__bin_snn_filt_SOURCES = util/matrix.c util/util_vector.c  util/util.c util/spikes_list.c  util/io.c filt/filt_funcs.c filt/optim_lbfgs.c snn_filt.c
__top_builddir__bin_snn_filt_CFLAGS = $(AM_CFLAGS)
__top_builddir__bin_snn_filt_LDADD = -llbfgs -lm


lib_LTLIBRARIES = $(top_builddir)/bin/libsnn_sim.la
__top_builddir__bin_libsnn_sim_la_SOURCES = $(MAIN_SOURCES) 
__top_builddir__bin_libsnn_sim_la_LDFLAGS = -version-info 0:1

