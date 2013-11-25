TARGET = test

SOURCES = test.c
CFC_DIR = ..\..
LIBS = $(CFC_DIR)\cserialfc.lib

all: $(TARGET).exe

$(TARGET).exe:$(SOURCES) $(HEADERS) $(LIBS)
  cl /D_WIN32_WINNT=0x0502 /W4 /MT $(SOURCES) $(LIBS) /I $(CFC_DIR) /I $(CFC_DIR)/src/ /Fe$@
  
clean:
  del *.exe