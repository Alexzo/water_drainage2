ifneq ($(wildcard config.make),)
    include config.make
endif

ifndef OF_ROOT
    OF_ROOT=../../../Developer/openFrameworks/macOS_0_11_2/Developer/openFrameworks/macOS_0_11_2/Developer/openFrameworks/macOS_0_11_2/Developer/openFrameworks/macOS_0_11_2
endif

include $(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk
