# the above include may override LKROOT and LKINC to allow external
# directories to be included in the build
-include lk_inc.mk

LKMAKEROOT ?= .
LKROOT ?= .
LKINC ?=
BUILDROOT ?= .
DEFAULT_PROJECT ?=
TOOLCHAIN_PREFIX ?=

# Variables for use with CodeSigner
CSLV_PATH := $(LKROOT)/tools/cslv
CSLV_32 := $(CSLV_PATH)/cslv_32
CSLV_64 := $(CSLV_PATH)/cslv_64

# variable for build server OS info
OS_INFO := $(findstring x86_64, $(shell uname -a))

################################################################################
# SoC configs								       #
################################################################################
ifeq ($(MAKECMDGOALS), maestro9820)
LK_PAD_SIZE := 1048576
SB_SIGN_TYPE := 3
SB_KEY_TYPE := 1
SB_RB_COUNT := 0
SIGNATURE_BIN := yes
else ifeq ($(MAKECMDGOALS), smdk9830)
LK_PAD_SIZE := 1048576
SB_SIGN_TYPE := 3
SB_KEY_TYPE := 1
SB_RB_COUNT := 0
SIGNATURE_BIN := yes
endif

# check if LKROOT is already a part of LKINC list and add it only if it is not
ifeq ($(filter $(LKROOT),$(LKINC)), )
LKINC := $(LKROOT) $(LKINC)
endif

# add the external path to LKINC
ifneq ($(LKROOT),.)
LKINC += $(LKROOT)/external
else
LKINC += external
endif

export LKMAKEROOT
export LKROOT
export LKINC
export BUILDROOT
export DEFAULT_PROJECT
export TOOLCHAIN_PREFIX

# veneer makefile that calls into the engine with lk as the build root
# if we're the top level invocation, call ourselves with additional args
_top:
	@$(MAKE) -C $(LKMAKEROOT) -rR -f $(LKROOT)/engine.mk $(addprefix -I,$(LKINC)) $(MAKECMDGOALS)
ifeq ($(SIGNATURE_BIN), yes)
	@$(LKROOT)/tools/Makepad_sb40 "build-"$(MAKECMDGOALS)/lk.bin $(LK_PAD_SIZE)
	@mv "build-"$(MAKECMDGOALS)/lk.bin "build-"$(MAKECMDGOALS)/lk.bin.tmp
ifeq ($(OS_INFO), x86_64)
	@echo "64 bit signer"
	@$(CSLV_64) -infile "build-"$(MAKECMDGOALS)/lk.bin.tmp -outfile "build-"$(MAKECMDGOALS)/lk.bin -sign_type $(SB_SIGN_TYPE) -key_type $(SB_KEY_TYPE) -rb_count $(SB_RB_COUNT) -dynamic_length no
else
	echo "32 bit signer"
	@$(CSLV_32) -infile "build-"$(MAKECMDGOALS)/lk.bin.tmp -outfile "build-"$(MAKECMDGOALS)/lk.bin -sign_type $(SB_SIGN_TYPE) -key_type $(SB_KEY_TYPE) -rb_count $(SB_RB_COUNT) -dynamic_length no
endif
	@rm "build-"$(MAKECMDGOALS)/lk.bin.tmp
endif

# If any arguments were provided, create a recipe for them that depends
# on the _top rule (thus calling it), but otherwise do nothing.
# "@:" (vs empty rule ";") prevents extra "'foo' is up to date." messages from
# being emitted.
$(MAKECMDGOALS): _top
	@:

.PHONY: _top
