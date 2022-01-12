CC       = g++
LDFLAGS  =
CPPFLAGS = -std=c++14 -g
AR       = ar -rc
RANLIB   = ranlib

BUILD_DIR=build
LIB_DIR=lib

# sources
pf_sources = PF_PageHandle.cpp PF_PrintError.cpp PF_FileHandle.cpp PF_Manager.cpp PF_BufferManger.cpp
sources    = main.cpp $(pf_sources)

# objects
pf_objects = $(addprefix $(BUILD_DIR)/, $(pf_sources:cpp=o))
objs       = $(addprefix $(BUILD_DIR)/, $(sources:.cpp=.o))

#libs
PF_LIB     = $(LIB_DIR)/libpf.a
LIBS       = $(PF_LIB)
LIBS_FLAGS = -lpf

redbase: $(objs) $(LIBS)
	$(CC) $(LDFLAGS) $(BUILD_DIR)/main.o -L $(LIB_DIR) $(LIBS_FLAGS) -o $@

$(PF_LIB): $(pf_objects)
	$(AR) $@ $(pf_objects)
	$(RANLIB) $@

-include $(objs:.o=.d)

$(BUILD_DIR)/%.d: %.cpp
	@set -e; rm -f $@; \
	$(CC) $(CPPFLAGS) -MM -MT $(@:.d=.o) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(objs): %.o:
	$(CC) $(CPPFLAGS) -c $< -o $@

.PHONY: clean print
clean:
	rm redbase $(objs) $(objs:.o=.d)

print:
	echo $(objs)