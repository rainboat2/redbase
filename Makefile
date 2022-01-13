CC       = g++
LDFLAGS  =
CPPFLAGS = -std=c++14 -g
AR       = ar -rc
RANLIB   = ranlib

BUILD_DIR=build
LIB_DIR=lib
TEST_DIR=test
TARGET_DIR=bin

# sources
PF_SOURCES   = PF_PageHandle.cpp PF_PrintError.cpp PF_FileHandle.cpp PF_Manager.cpp PF_BufferManger.cpp
TEST_SOURCES = PF_Test.cpp
SOURCES      = main.cpp $(PF_SOURCES) ${TEST_SOURCES}

# objects
PF_OBJECTS   = $(addprefix $(BUILD_DIR)/, $(PF_SOURCES:cpp=o))
TEST_OBJECTS = $(addprefix ${BUILD_DIR}/, ${TEST_SOURCES:cpp=o})
OBJECTS      = $(addprefix $(BUILD_DIR)/, $(SOURCES:.cpp=.o))

#libs
PF_LIB     = $(LIB_DIR)/libpf.a
TEST_LIB   = ${LIB_DIR}/libredbase_test.a
LIBS       = $(PF_LIB)
LIBS_FLAGS = -lpf  -lgtest -lgtest_main -pthread

# targets
TESTERS = $(TEST_SOURCES:.cpp=)

all: redbase $(TESTERS)

redbase: $(OBJECTS) $(LIBS)
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(BUILD_DIR)/main.o -L $(LIB_DIR) $(LIBS_FLAGS) -o $(TARGET_DIR)/$@

$(TESTERS): % : $(BUILD_DIR)/%.o 
	$(CC) $(CPPFLAGS) $(LDFLAGS) $< -L $(LIB_DIR) $(LIBS_FLAGS) -o $(TARGET_DIR)/$@

# generate library file
$(PF_LIB): $(PF_OBJECTS)
	$(AR) $@ $(PF_OBJECTS)
	$(RANLIB) $@

$(TEST_LIB): $(TEST_OBJECTS)
	$(AR) $@ $(TEST_OBJECTS)
	$(RANLIB) $@


# generate object file
-include $(OBJECTS:.o=.d)

$(BUILD_DIR)/%.d: %.cpp
	@set -e; rm -f $@; \
	$(CC) $(CPPFLAGS) -MM -MT $(@:.d=.o) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJECTS): %.o:
	$(CC) $(CPPFLAGS) -c $< -o $@

.PHONY: clean print run_test
clean:
	rm redbase tester $(BUILD_DIR)/*.d  $(BUILD_DIR)/*.o lib/*.a
print:
	echo $(OBJECTS)
run_test: $(TESTERS)
	bin/PF_Test