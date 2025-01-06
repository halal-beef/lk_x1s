ifndef ARCH_arm64_TOOLCHAIN_INCLUDED
ARCH_arm64_TOOLCHAIN_INCLUDED := 1

ifndef ARCH_arm64_TOOLCHAIN_PREFIX
	ifeq ($(TOOLCHAIN_PREFIX),)
		ARCH_arm64_TOOLCHAIN_PREFIX := aarch64-linux-gnu-
		FOUNDTOOL=$(shell which $(ARCH_arm64_TOOLCHAIN_PREFIX)gcc)
		ifeq ($(FOUNDTOOL),)
			ARCH_arm64_TOOLCHAIN_PREFIX := aarch64-linux-gnu-
			FOUNDTOOL=$(shell which $(ARCH_arm64_TOOLCHAIN_PREFIX)gcc)
			ifeq ($(FOUNDTOOL),)
				ARCH_arm64_TOOLCHAIN_PREFIX := aarch64-linux-gnu-
				FOUNDTOOL=$(shell which $(ARCH_arm64_TOOLCHAIN_PREFIX)gcc)
				ifeq ($(FOUNDTOOL),)
					$(error cannot find toolchain, please set ARCH_arm64_TOOLCHAIN_PREFIX or add it to your path)
				endif
			endif
		endif
	else
		ARCH_arm64_TOOLCHAIN_PREFIX := $(TOOLCHAIN_PREFIX)
		FOUNDTOOL=$(shell which $(ARCH_arm64_TOOLCHAIN_PREFIX)gcc)
		ifeq ($(FOUNDTOOL),)
			$(error cannot find toolchain, please set valid TOOLCHAIN_PREFIX.)
		endif
	endif
endif

#ARCH_arm64_COMPILEFLAGS := -mgeneral-regs-only -DWITH_NO_FP=1

endif
