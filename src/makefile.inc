_LNG=$(LANGUAGE)
_INX=.
STAMP=stampinf -f $@ -a $(_BUILDARCH) -v 3.0.0.0 -k $(KMDF_VERSION_MAJOR).$(KMDF_VERSION_MINOR)



$(OBJ_PATH)\$(O)\$(INF_NAME).inf: $(_INX)\$(INF_NAME).inx
    copy $(_INX)\$(@B).inx $@
    $(STAMP)

$(OBJ_PATH)\$(O)\$(INF_NAME2).inf: $(_INX)\$(INF_NAME2).inx
    copy $(_INX)\$(@B).inx $@
    $(STAMP)
