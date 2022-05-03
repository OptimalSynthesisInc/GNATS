#
# Makefile
#

# Subprojects to be built.
# if you add a subproject, be sure to list it here, if appropriate.
subdirs=libastar \
	libairport_layout \
	libadb \
	libcommon \
	libcuda_compat \
	libnats_data \
	libhuman_error \
	libfp \
	libgeomutils \
	liblektor \
	libpilot \
	librg \
	libtg \
	libtrx \
	libwind \
	wind_tool


dbg_subdirs=libcg liblp lp

DIST_NAME=optimize-atm

MAJOR=1
MINOR=2

# Build CPU version by default.  This can be overridden by supplying
# USE_GPU=1 to the make command. Note: liblpss and lpss are always
# built with USE_GPU=1.
USE_GPU=0

srcdir=src
distdir=dist/$(DIST_NAME)-$(MAJOR).$(MINOR)

.PHONY: dist

all: deps

libastar:
	$(MAKE) -C $(srcdir)/libastar/src

libairport_layout:
	$(MAKE) -C $(srcdir)/libairport_layout/src

libadb:
	$(MAKE) -C $(srcdir)/libbadb/src

libcommon:
	$(MAKE) -C $(srcdir)/libcommon/src

libcontroller:
	$(MAKE) -C $(srcdir)/libcontroller/src
	
libcuda_compat:
	$(MAKE) -C $(srcdir)/libcuda_compat/src USE_GPU=$(USE_GPU)

libnats_data:
	$(MAKE) -C $(srcdir)/libnats_data/src

libhuman_error:
	$(MAKE) -C $(srcdir)/libhuman_error/src

libfp:
	$(MAKE) -C $(srcdir)/libfp/src

libgeomutils:
	$(MAKE) -C $(srcdir)/libgeomutils/src

liblektor:
	$(MAKE) -C $(srcdir)/liblektor/src

libpilot:
	$(MAKE) -C $(srcdir)/libpilot/src

librg:
	$(MAKE) -C $(srcdir)/librg/src USE_GPU=$(USE_GPU)
	
libtg:
	$(MAKE) -C $(srcdir)/libtg/src USE_GPU=$(USE_GPU)

libtrx:
	$(MAKE) -C $(srcdir)/libtrx/src

libwind:
	$(MAKE) -C $(srcdir)/libwind/src

wind_tool:
	$(MAKE) -C $(srcdir)/wind_tool/src




# Make dependencies
deps:
	$(MAKE) -C $(srcdir)/libcommon/src
	$(MAKE) -C $(srcdir)/libuser/src
	$(MAKE) -C $(srcdir)/libhuman_error/src
	$(MAKE) -C $(srcdir)/libcontroller/src
	$(MAKE) -C $(srcdir)/libpilot/src
	$(MAKE) -C $(srcdir)/liblektor/src
	$(MAKE) -C $(srcdir)/libnats_data/src
	$(MAKE) -C $(srcdir)/libwind/src
	$(MAKE) -C $(srcdir)/wind_tool/src USE_GPU=$(USE_GPU)
	$(MAKE) -C $(srcdir)/libastar/src
	$(MAKE) -C $(srcdir)/libadb/src
	$(MAKE) -C $(srcdir)/libairport_layout/src
	$(MAKE) -C $(srcdir)/libcuda_compat/src
	$(MAKE) -C $(srcdir)/libfp/src
	$(MAKE) -C $(srcdir)/libgeomutils/src
	$(MAKE) -C $(srcdir)/libtrx/src
	$(MAKE) -C $(srcdir)/librg/src
	$(MAKE) -C $(srcdir)/libtg/src

# Clean sources
clean: $(addprefix clean-,$(subdirs))

# Clean sources of subprojects
clean-%:
	@echo "Cleaning $(subst clean-,,$@)"
	$(MAKE) -C $(srcdir)/$(subst clean-,,$@)/src clean

# Clean sources and dists, remove all symlinks from lib, include, share
clobber: $(addprefix clobber-,$(subdirs))
	rm -rf $(distdir)
	rm -rf dist
	$(shell find lib -maxdepth 1 -type l | xargs rm -rf)
	$(shell find include -maxdepth 1 -type l | xargs rm -rf)
	$(shell find include -mindepth 1 -type d -not -name '.svn' | xargs rm -rf)
	$(shell find share -maxdepth 1 -type l | xargs rm -rf)
	$(shell find bin -maxdepth 1 -type l | xargs rm -rf)

# Clean sources and dists of sub-projects
clobber-%:
	@echo "Clobbering $(subst clobber-,,$@)"
	$(MAKE) -C $(srcdir)/$(subst clobber-,,$@)/src clean
	$(MAKE) -C $(srcdir)/$(subst clobber-,,$@)/src distclean

# Create dist directory structure
init_dist:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/lib
	mkdir -p $(distdir)/share
	mkdir -p $(distdir)/include

init_dist_subdirs: init_dist
	$(foreach i, $(shell find include -mindepth 1 -maxdepth 1 -type d), $(shell mkdir -p $(distdir)/$(i)))

# Copy subproject dist files to top-level dist, follow links
copy_dist: $(addprefix dist-,$(subdirs)) init_dist_subdirs
	$(foreach i, $(shell find lib -name '*.so'), $(shell cp `readlink -f $(i)` $(distdir)/lib/.))
	$(foreach i, $(shell find -L share -mindepth 1 -maxdepth 1 -type d), $(shell cp -r `readlink -f $(i)` $(distdir)/share/.))
	$(foreach i, $(shell find include -mindepth 1 -maxdepth 1 -type d), $(foreach link, $(shell find $(i) -mindepth 1 -maxdepth 1 -type l), $(shell cp `readlink -f $(link)` $(distdir)/$(link) )))
	$(foreach i, $(shell find bin -type l), $(shell cp `readlink -f $(i)` $(distdir)/bin/.))
	cp optimize_atm.ini $(distdir)/.
	cp optimize_atm $(distdir)/.

copy_dist_dbg: $(addprefix dist-,$(dbg_subdirs)) init_dist_subdirs
	$(foreach i, $(shell find lib -name '*.so'), $(shell cp `readlink -f $(i)` $(distdir)/lib/.))
	$(foreach i, $(shell find -L share -mindepth 1 -maxdepth 1 -type d), $(shell cp -r `readlink -f $(i)` $(distdir)/share/.))
	$(foreach i, $(shell find include -mindepth 1 -maxdepth 1 -type d), $(foreach link, $(shell find $(i) -mindepth 1 -maxdepth 1 -type l), $(shell cp `readlink -f $(link)` $(distdir)/$(link) )))
	$(foreach i, $(shell find bin -type l), $(shell cp `readlink -f $(i)` $(distdir)/bin/.))
	cp optimize_atm.ini $(distdir)/.
	cp optimize_atm $(distdir)/.

# Remove unwanted files from the dist (.svn folders)
dist_remove_svn: copy_dist init_dist
	$(shell find $(distdir) -name '.svn' | xargs rm -rf)

# Archive dist
#dist: copy_dist
dist: init_dist copy_dist dist_remove_svn
#	cd dist && tar cvfz $(DIST_NAME)-$(MAJOR).$(MINOR).tar.gz $(DIST_NAME)-$(MAJOR).$(MINOR)

dbg_dist: init_dist copy_dist_dbg dist_remove_svn


# Build subproject dists
dist-%:
	@echo "Building dist for $(subst dist-,,$@)"
	$(MAKE) -C $(srcdir)/$(subst dist-,,$@)/src dist
