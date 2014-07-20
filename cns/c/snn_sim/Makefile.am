# what flags you want to pass to the C compiler & linker
AM_CFLAGS = -Wall -std=gnu99 -pthread  -O3  #-g  
#AM_CFLAGS = -Wall -std=gnu99 -pthread -O0 -g  
CFLAGS=
LDADD= -lm
# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
#bin_PROGRAMS = srm_sim srm_sim_ut
#bin_PROGRAMS = $(top_builddir)/bin/srm_sim $(top_builddir)/bin/srm_sim_ut
#bin_PROGRAMS = $(top_builddir)/bin/snn_sim
bin_PROGRAMS = $(top_builddir)/bin/snn_sim $(top_builddir)/bin/snn_prep $(top_builddir)/bin/snn_calc $(top_builddir)/bin/snn_postproc
__top_builddir__bin_snn_sim_SOURCES = util/matrix.c util/util_vector.c ./util/ini.c ./util/matrix.h util/util.c util/spikes_list.c ./util/util_dlink_list.c sim/netsim.c neuron_funcs.c  constants.c util/io.c util/mem.c sim/runtime.c sim/configure.c sim/serialize.c ./layers/layer_poisson.c sim/sim.c learn/optim.c learn/res_stdp.c learn/simple_stdp.c layers/layer_wta.c layers/layer_adapt.c layers/layer_wtaadapt.c snn_sim.c 
__top_builddir__bin_snn_prep_SOURCES = util/matrix.c util/util_vector.c ./util/ini.c ./util/matrix.h util/util.c util/spikes_list.c  constants.c util/io.c layers/adex.c prep/prepare_ts.c snn_prep.c 
__top_builddir__bin_snn_calc_SOURCES = util/matrix.c util/util_vector.c ./util/ini.c ./util/matrix.h util/util.c util/spikes_list.c  util/io.c snn_calc.c calc.c
__top_builddir__bin_snn_postproc_SOURCES = util/matrix.c util/util_vector.c ./util/ini.c ./util/matrix.h util/util.c util/spikes_list.c  util/io.c postproc.c snn_postproc.c
