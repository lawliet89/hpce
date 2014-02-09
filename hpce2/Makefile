# Variables/settings
SRC_URL = http://cas.ee.ic.ac.uk/people/dt10/teaching/2013/hpce/cw2
LAME = lame-3.99.5
SOX = sox-14.4.1

# "Dummy Targets"
tools: local/bin/sox local/bin/lame

# Download Targets
packages/$(SOX).tar.gz: dir
	curl $(SRC_URL)/$(SOX).tar.gz -o packages/$(SOX).tar.gz
packages/$(LAME).tar.gz: dir
	curl $(SRC_URL)/$(LAME).tar.gz -o packages/$(LAME).tar.gz

# Untar Targets

# Note: This rule will always run because the file untared from the tarball
# is older than the tarball
# Use make -d | less and you will see
# Prerequisite `packages/$(SOX).tar.gz' is newer than target
# `build/$(SOX)/configure'.
build/$(SOX)/configure: packages/$(SOX).tar.gz
	cd build && tar -xzf ../packages/$(SOX).tar.gz
build/$(LAME)/configure: packages/$(LAME).tar.gz
	cd build && tar -xzf ../packages/$(LAME).tar.gz

# Configure Targets
build/$(SOX)/Makefile: build/$(SOX)/configure
	cd build/$(SOX) && ./configure --prefix=`pwd`/../../local
build/$(LAME)/Makefile: build/$(LAME)/configure
	cd build/$(LAME) && ./configure --prefix=`pwd`/../../local

# Build
build/$(SOX)/src/sox: build/$(SOX)/Makefile
	cd build/$(SOX) && $(MAKE)
build/$(LAME)/src/lame: build/$(LAME)/Makefile
	cd build/$(LAME) && $(MAKE)

# Install
local/bin/sox: build/$(SOX)/src/sox
	cd build/$(SOX) && $(MAKE) install

local/bin/lame: build/$(LAME)/src/lame
	cd build/$(LAME) && $(MAKE) install

# Housekeeping Targets
clean:
	rm -rf packages/* build/* local/*

dir: packages build local tmp
packages:
	mkdir -p packages
build:
	mkdir -p build
local:
	mkdir -p local
tmp:
	mkdir -p tmp

