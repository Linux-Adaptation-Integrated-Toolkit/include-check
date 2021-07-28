LLVM_CONFIG?=llvm-config

ifndef VERBOSE
QUIET:=@
endif

SRC_DIR?=$(PWD)
LDFLAGS+=$(shell $(LLVM_CONFIG) --ldflags)
COMMON_FLAGS=-Wall -Wextra
CXXFLAGS+=$(COMMON_FLAGS) $(shell $(LLVM_CONFIG) --cxxflags) -fno-rtti
CPPFLAGS+=$(shell $(LLVM_CONFIG) --cppflags) -I$(SRC_DIR)
CLANGLIBS=\
  -Wl,--start-group\
  -lclang\
  -lclangFrontend\
  -lclangDriver\
  -lclangSerialization\
  -lclangParse\
  -lclangSema\
  -lclangAnalysis\
  -lclangEdit\
  -lclangAST\
  -lclangLex\
  -lclangBasic\
  -Wl,--end-group
OTHERLIBS=\
	-lcjson
LLVMLIBS=$(shell $(LLVM_CONFIG) --libs)
SYSTEMLIBS=$(shell $(LLVM_CONFIG) --system-libs)

PROJECT=deepin-scanlib
PROJECT_OBJECTS=deepin-scanlib.o

default: $(PROJECT)

$(PROJECT_OBJECTS) : $(SRC_DIR)/src/deepin-scanlib.cpp
	@echo Compiling $*.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $(SRC_DIR)/src/$(PROJECT_OBJECTS)

$(PROJECT) : $(SRC_DIR)/src/$(PROJECT_OBJECTS)
	@echo Linking $@
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $^ $(CLANGLIBS) $(LLVMLIBS) $(OTHERLIBS) $(SYSTEMLIBS)

clean::
	$(QUIET)rm -f $(PROJECT) $(SRC_DIR)/src/$(PROJECT_OBJECTS)
