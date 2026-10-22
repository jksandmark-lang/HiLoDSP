# Project Name
TARGET = HiLoDSP

# Sources
CPP_SOURCES = HiLoDSP.cpp CrossoverProcessor.cpp MediaPlayer.cpp AudioClock.cpp

# FatFs Unicode tables (ff_convert / ff_wtoupper) are not in libdaisy.a.
C_SOURCES = ../../libDaisy/Middlewares/Third_Party/FatFs/src/option/unicode.c

# Library Locations
LIBDAISY_DIR = ../../libDaisy/
DAISYSP_DIR = ../../DaisySP/

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
