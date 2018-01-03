PACKAGES       = braids/test stmlib/utils braids braids/dexed braids/vocalist braids/vocalist/sam

VPATH          = $(PACKAGES)

TARGET         = braids_test
BUILD_ROOT     = build/
BUILD_DIR      = $(BUILD_ROOT)$(TARGET)/
CC_FILES       = dexed_audio_processor.cc \
		dx7note.cc \
		dexed_lut.cc \
		EngineMkI.cc \
		fm_core.cc \
		freqlut.cc \
		lfo.cc \
		braids_test.cc \
		quantizer.cc \
		resources.cc \
		random.cc \
		pitchenv.cc \
		env.cc \
		fm_op_kernel.cc \
		vocalist.cc \
		sin.cc \
		reciter.cc sam.cc render.cc debug.cc processframes.cc \
		createtransitions.cc rendertabs.cc wordlist.cc

OBJ_FILES      = $(CC_FILES:.cc=.o)
OBJS           = $(patsubst %,$(BUILD_DIR)%,$(OBJ_FILES)) $(STARTUP_OBJ)
DEPS           = $(OBJS:.o=.d)
DEP_FILE       = $(BUILD_DIR)depends.mk

all:  braids_test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)%.o: %.cc
	g++ -c -DTEST -g -Wall -Werror -Wno-narrowing -Wno-unused-variable -I. $< -o $@

$(BUILD_DIR)%.d: %.cc
	g++ -MM -DTEST -I. $< -MF $@ -MT $(@:.d=.o)

braids_test:  $(OBJS)
	g++ -o $(TARGET) $(OBJS)

depends:  $(DEPS)
	cat $(DEPS) > $(DEP_FILE)

$(DEP_FILE):  $(BUILD_DIR) $(DEPS)
	cat $(DEPS) > $(DEP_FILE)

include $(DEP_FILE)
