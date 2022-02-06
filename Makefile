CC       = g++
CPPFLAGS = -std=c++14 -g -Wall
LDFLAGS  = -L $(LIB_DIR)
LIBS     = -ltool -lpf -lrm  -lix -lsm -lgtest -lgtest_main -pthread
AR       = ar -rc
RANLIB   = ranlib

BUILD_DIR   = build
LIB_DIR     = $(BUILD_DIR)/lib
OBJECTS_DIR = $(BUILD_DIR)/object
TARGET_DIR  = $(BUILD_DIR)/bin
DIRS        = $(BUILD_DIR) $(LIB_DIR) $(OBJECTS_DIR) $(TARGET_DIR)

# utilites
UTILITES = redbase.cpp dbcreate.cpp dbdestroy.cpp

# sources
TOOL_SOURCES = RedbaseComparator.cpp PrintError.cpp BitMapWapper.cpp
PF_SOURCES   = PF_PageHandle.cpp PF_FileHandle.cpp PF_Manager.cpp PF_BufferManager.cpp
RM_SOURCES   = RM_RID.cpp RM_Manager.cpp RM_Record.cpp RM_FileHandle.cpp RM_FileScan.cpp
IX_SOURCES   = IX_Manager.cpp IX_IndexHandle.cpp IX_BNodeWapper.cpp IX_IndexScan.cpp IX_BBucketListWapper.cpp IX_BBucketWapper.cpp
SM_SOURCES   = SM_Manager.cpp
TEST_SOURCES = PF_Test.cpp RM_Test.cpp IX_Test.cpp
SOURCES      = $(TOOL_SOURCES) $(PF_SOURCES) $(RM_SOURCES) $(TEST_SOURCES) $(IX_SOURCES) $(SM_SOURCES)

# objects
TOOL_OBJECTS = $(addprefix $(OBJECTS_DIR)/, $(TOOL_SOURCES:cpp=o))
PF_OBJECTS   = $(addprefix $(OBJECTS_DIR)/, $(PF_SOURCES:cpp=o))
RM_OBJECTS   = $(addprefix $(OBJECTS_DIR)/, $(RM_SOURCES:cpp=o))
IX_OBJECTS   = $(addprefix $(OBJECTS_DIR)/, $(IX_SOURCES:cpp=o))
SM_OBJECTS   = $(addprefix $(OBJECTS_DIR)/, $(SM_SOURCES:cpp=o))
TEST_OBJECTS = $(addprefix ${OBJECTS_DIR}/, ${TEST_SOURCES:cpp=o})
OBJECTS      = $(addprefix $(OBJECTS_DIR)/, $(SOURCES:.cpp=.o))

#libs
TOOL_LIB      = $(LIB_DIR)/libtool.a
PF_LIB        = $(LIB_DIR)/libpf.a
RM_LIB        = $(LIB_DIR)/librm.a
IX_LIB        = $(LIB_DIR)/libix.a
SM_LIB        = $(LIB_DIR)/libsm.a
READBASE_LIBS = $(TOOL_LIB) $(PF_LIB) $(RM_LIB) $(IX_LIB) $(SM_LIB)

# generate excutable file
all: make_dirs $(UTILITES:.cpp=) redbaseTest

$(UTILITES:.cpp=): % : %.cpp $(READBASE_LIBS)
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(LIBS) $< -o $(TARGET_DIR)/$@

redbaseTest: $(OBJECTS) ${READBASE_LIBS}
	$(CC) $(CPPFLAGS) $(TEST_OBJECTS) $(LDFLAGS) $(LIBS) -o $(TARGET_DIR)/$@

# generate library file
$(TOOL_LIB): $(TOOL_OBJECTS)
	$(AR) $@ $(TOOL_OBJECTS)
	$(RANLIB) $@

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

$(OBJECTS_DIR)/%.d: %.cpp
	@set -e; \
	mkdir -p $(OBJECTS_DIR); \
	rm -f $@; \
	$(CC) $(CPPFLAGS) -MM -MT $(@:.d=.o) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJECTS): %.o:
	$(CC) $(CPPFLAGS) -c $< -o $@

# phony target
.PHONY: clean print run_test make_dirs
clean:
	rm -rf $(DIRS)
print:
	echo $(OBJECTS)
run_test: redbaseTest
	$(TARGET_DIR)/redbaseTest $(cmd_args)
make_dirs:
	mkdir -p $(DIRS)