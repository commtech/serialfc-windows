TARGET = cppserialfc

SOURCES = src/serialfc.cpp src/sys_exception.cpp
HEADERS = src/serialfc.hpp src/sys_exception.hpp ../c/src/serialfc.h
CFC_DIR = ..\c
LIBS = $(CFC_DIR)\cserialfc.lib
LIBSD = $(CFC_DIR)\cserialfcd.lib

all: $(TARGET).dll 
debug: $(TARGET)d.dll

$(TARGET).dll:$(SOURCES) $(HEADERS) $(LIBS)
  cl $(SOURCES) /I$(CFC_DIR)\src\ $(LIBS) /EHsc /Fe$@ /LD

$(TARGET)d.dll:$(SOURCES) $(HEADERS) $(LIBSD)
  cl $(SOURCES) /I$(CFC_DIR)\src\ $(LIBSD) /EHsc /Fe$@ /LDd /Zi
  
$(LIBS):
  pushd $(CFC_DIR) & nmake & popd
  
$(LIBSD):
  pushd $(CFC_DIR) & nmake DEBUG & popd
  
clean:
  del cppserialfc* *.obj