TARGET = cserialfc

SOURCES = src/serialfc.c
HEADERS = src/serialfc.h

all: $(TARGET).dll
debug: $(TARGET)d.dll

$(TARGET).dll:$(SOURCES) $(HEADERS)
  cl $(SOURCES) /Fe$@ /LD

$(TARGET)d.dll:$(SOURCES) $(HEADERS)
  cl $(SOURCES) /Fe$@ /LDd /Zi
  
clean:
  del cserialfc* *.obj