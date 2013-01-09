TARGET = netserialfc

SOURCES = src\Serialfc.cs
CFC_DIR = ..\c
LIBS = $(CFC_DIR)\cserialfc.lib
LIBSD = $(CFC_DIR)\cserialfcd.lib

all: $(TARGET).dll 
debug: $(TARGET)d.dll

$(TARGET).dll:$(SOURCES) $(LIBS)
  csc /linkresource:$(CFC_DIR)\cserialfc.dll /t:library /out:$@ $(SOURCES) /platform:x86  

$(TARGET)d.dll:$(SOURCES) $(LIBSD)
  csc /linkresource:$(CFC_DIR)\cserialfcd.dll /t:library /out:$@ $(SOURCES) /platform:x86
  
$(LIBS):
  pushd $(CFC_DIR) & nmake & popd
  
$(LIBSD):
  pushd $(CFC_DIR) & nmake DEBUG & popd
  
clean:
  del netserialfc*