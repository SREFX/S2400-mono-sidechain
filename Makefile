NAME = MonoSC
FILES_DSP = MonoSC.cpp

# We are building an effect, so we use the plugin template
include ../../Makefile.plugins.mk

# Link against the framework
all: lv2
		@echo "---manually forcing TTL generation---"
		@../../utils/lv2_ttl_generator/lv2_ttl_generator ../../bin/MonoSC.lv2/MonoSC.so
		@cp *.ttl ../../bin/MonoSC.lv2
		@echo "Done! ttls copied to plugin directory"