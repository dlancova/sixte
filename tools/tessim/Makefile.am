AM_CPPFLAGS =-I@top_srcdir@/libsixt
AM_CFLAGS =-I@top_srcdir@/extlib/progressbar/include

########## DIRECTORIES ###############

# Directory where to install the PIL parameter files.
pfilesdir=$(pkgdatadir)/pfiles
dist_pfiles_DATA=tessim.par

############ BINARIES #################

# The following line lists the programs that should be created and stored
# in the 'bin' directory.
bin_PROGRAMS=tessim

tessim_SOURCES=tessim.c tessim.h tessim_datastream.c tessim_tesrecord.c tessim_impactlist.c tes_simulation.c tessim_trigger.c tes_models.c tessim_solvers.c tessim_bbfb.c
tessim_LDADD =@top_builddir@/libsixt/libsixt.la @top_builddir@/extlib/progressbar/libprogressbar.la
