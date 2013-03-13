TARGET = loop

SOURCES = loop.c
HEADERS = "$(VXIPNPPATH)WinNT\include\visa.h"
LIBS = "$(VXIPNPPATH)WinNT\lib\msc\visa32.lib"

all: $(TARGET).exe

$(TARGET).exe:$(SOURCES) $(HEADERS) $(LIBS)
  cl /W4 $(SOURCES) /I"%VXIPNPPATH%WinNT\include" $(LIBS) /Fe$@
  
clean:
  del *.exe