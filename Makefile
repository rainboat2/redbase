CC       = g++
LDFLAGS  = -L $(LIB_DIR)
LIBS     = -lpf -lrm  -lix -lsm -lgtest -lgtest_main -pthread
CPPFLAGS = -std=c++14 -g -Wall
AR       = ar -rc
RANLIB   = ranlib

BUILD_DIR  = build
LIB_DIR    = lib
TEST_DIR   = test
TARGET_DIR = bin

# utilites
UTILITES = redbase.cpp dbcreate.cpp

# sources
PF_SOURCES   = PF_PageHandle.cpp PrintError.cpp PF_FileHandle.cpp PF_Manager.cpp PF_BufferManager.cpp RedbaseComparator.cpp
RM_SOURCES   = BitMapWapper.cpp RM_RID.cpp RM_Manager.cpp RM_Record.cpp RM_FileHandle.cpp RM_FileScan.cpp
IX_SOURCES   = IX_Manager.cpp IX_IndexHandle.cpp IX_BNodeWapper.cpp IX_IndexScan.cpp
SM_SOURCES   = SM_Manager.cpp
TEST_SOURCES = PF_Test.cpp RM_Test.cpp IX_Test.cpp
SOURCES      = $(PF_SOURCES) $(RM_SOURCES) $(TEST_SOURCES) $(IX_SOURCES) $(SM_SOURCES)

# objects
PF_OBJECTS   = $(addprefix $(BUILD_DIR)/, $(PF_SOURCES:cpp=o))
RM_OBJECTS   = $(addprefix $(BUILD_DIR)/, $(RM_SOURCES:cpp=o))
IX_OBJECTS   = $(addprefix $(BUILD_DIR)/, $(IX_SOURCES:cpp=o))
SM_OBJECTS   = $(addprefix $(BUILD_DIR)/, $(SM_SOURCES:cpp=o))
TEST_OBJECTS = $(addprefix ${BUILD_DIR}/, ${TEST_SOURCES:cpp=o})
OBJECTS      = $(addprefix $(BUILD_DIR)/, $(SOURCES:.cpp=.o))

#libs
PF_LIB        = $(LIB_DIR)/libpf.a
RM_LIB        = $(LIB_DIR)/librm.a
IX_LIB        = $(LIB_DIR)/libix.a
SM_LIB        = $(LIB_DIR)/libsm.a
READBASE_LIBS = $(PF_LIB) $(RM_LIB) $(IX_LIB) $(SM_LIB)

all: $(UTILITES:.cpp=) redbaseTest

$(UTILITES:.cpp=): % : %.cpp $(READBASE_LIBS)
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(LIBS) $< -o $(TARGET_DIR)/$@

redbaseTest: $(OBJECTS) ${READBASE_LIBS}
	$(CC) $(CPPFLAGS) $(TEST_OBJECTS) $(LDFLAGS) $(LIBS) -o $(TARGET_DIR)/$@

# generate library file
$(PF_LIB): $(PF_OBJECTS)
	$(AR) $@ $(PF_OBJECTS)
	$(RANLIB) $@

$(RM_LIB): $(RM_OBJECTS)
	$(AR) $@ $(RM_OBJECTS)
	$(RANLIB) $@

$(IX_LIB): $(IX_OBJECTS)
	$(AR) $@ $(IX_OBJECTS)
	$(RANLIB) $@

$(SM_LIB): $(SM_OBJECTS)
	$(AR) $@ $(SM_OBJECTS)
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
	rm -rf bin/* $(BUILD_DIR)/*.d*  $(BUILD_DIR)/*.o lib/*.a
print:
	echo $(OBJECTS)
run_test: redbaseTest
	bin/redbaseTest $(cmd_args)