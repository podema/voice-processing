

SHELL   = /bin/sh

#------------------------------------------------------------------------------------
# TO DO: Add the directories to "make" (directories where where something must be done.)
MODULES :=make_wizard pav derivates gmm scripts #make_wizard mfcc derivates gmm scripts #etc...
#------------------------------------------------------------------------------------

.PHONY: help all debug release clean clean_debug clean_release clean_all $(MODULES)


help:
	@echo " "
	@echo "------------------------------------------------------------------------"
	@echo "Use:"
	@echo '  make release       : create "bin-&-lib"  release version'
	@echo '  make debug         : create "bin-&-lib"  debug version '
	@echo "  make all           : make debug and release  "
	@echo " "
	@echo '  make clean_release : remove the "release" intermediate files'
	@echo '  make clean_debug   : remove the "debug"   intermediate files'
	@echo '  make clean         : make clean_debug and clean_release'
	@echo " "
	@echo '  make clean_all     : remove also the "bin-&-lib" for both '
	@echo '                       debug and release'
	@echo "------------------------------------------------------------------------"
	@echo " "



all debug release clean clean_debug clean_release clean_all : $(MODULES)

$(MODULES):
	$(MAKE) -C $@ $(MAKECMDGOALS)


# If the work must continue not in the subdirectory but in some inner places,
# you can use as in the following example:

# $(MODULES):
#	$(MAKE) -C $@/src $(MAKECMDGOALS)
