#############################################
# Setup some basic definitions, so we can change the in one place if nesc.

SHELL=/bin/bash

SOX=../local/bin/sox
LAME=../local/bin/lame
MP3_FILE_SRC=../audio/mp3_file_src.sh
CORRELATION=../audio/correlation
FIR_FILTER=../audio/fir_filter

#############################################
# Build up the three dimensions of explorations

# Want to use all mp3s from the input directory
INPUTS:=$(wildcard inputs/*.mp3)
INPUTS:=$(basename $(notdir $(INPUTS) ) )

# Consider all coefficient sets in the coeffs directory
FILTERS:= $(wildcard coeffs/*.csv)
FILTERS:= $(basename $(notdir $(FILTERS)))

# Test them with a selection of window sizes
WINDOWS:=1 2 4 8 16 32 64 128 256 512 1024 2048 4096

# Some helpers, so for example ``make show_filters'' will list the current coefficients

show_filters :
	echo $(FILTERS)

show_inputs :
	echo $(INPUTS)
	
show_windows :
	echo $(WINDOWS)


#############################################
# Apply filters to mp3s

# This is a template that applies a given input mp3 identified by $(1), into 
#  a wav file filtered with coeffient set $(2)
define template_filter =

working/$(1)_$(2).wav : inputs/$(1).mp3 coeffs/$(2).csv
	$(MP3_FILE_SRC) inputs/$(1).mp3 \
		| $(FIR_FILTER) coeffs/$(2).csv \
		> working/$(1)_$(2).wav
		
endef

# Expand the template for every combination of INPUTS x FILTERS
$(foreach i,$(INPUTS),$(foreach f,$(FILTERS),$(eval $(call template_filter,$(i),$(f)))))


#############################################
# Apply analysis to filtered wavs

# A second template that analyses a wav file containint m3 $(1) filtered by coeffs $(2),
# using a window of size $(3)
define template_analyse= 

# Each output will be one line of a csv file containing the mp3, filter, and window,
#  plus the resulting correlation for that triple
working/$(1)_$(2)_$(3).csv : working/$(1)_$(2).wav
	
	echo -n "$(1), $(2), $(3), " > working/$(1)_$(2)_$(3).csv
	$(CORRELATION)	\
		<(cat working/$(1)_$(2).wav) \
		<( $(MP3_FILE_SRC) inputs/$(1).mp3 ) \
		$(3) \
	>> working/$(1)_$(2)_$(3).csv
	
endef

# Expand the template over all combinations of INPUTS x FILTERS x WINDOWS
$(foreach i,$(INPUTS),$(foreach f,$(FILTERS),$(foreach w,$(WINDOWS),$(eval $(call template_analyse,$(i),$(f),$(w))))))


###############################################
# Determine the set of things to build, and the final output

# A list of all csv files produces over INPUTS x FILTERS x WINDOWS
ALL_CSV = $(foreach i,$(INPUTS),$(foreach f,$(FILTERS),$(foreach w,$(WINDOWS), working/$(i)_$(f)_$(w).csv  )))

# Combine the rows into one large csv file
all.csv : $(ALL_CSV)
	cat $(ALL_CSV) > $@
