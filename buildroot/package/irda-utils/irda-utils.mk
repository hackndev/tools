#############################################################
#
# modutils
#
#############################################################
IRDA_UTILS_SOURCE=irda-utils-0.9.17-pre3.tar.gz
IRDA_UTILS_SITE=ftp://ftp.sourceforge.net/pub/sourceforge/i/ir/irda/
IRDA_UTILS_DIR1=$(BUILD_DIR)/irda-utils-0.9.17-pre3
IRDA_UTILS_DIR2=$(TOOL_BUILD_DIR)/irda-utils-0.9.17-pre3
IRDA_UTILS_BINARY=irattach/irattach
IRDA_UTILS_TARGET_BINARY=$(TARGET_DIR)/sbin/$(IRDA_UTILS_BINARY)

STRIPPROG=$(STRIP)

$(DL_DIR)/$(IRDA_UTILS_SOURCE):
	$(WGET) -P $(DL_DIR) $(IRDA_UTILS_SITE)/$(IRDA_UTILS_SOURCE)

#############################################################
#
# build modutils for use on the target system
#
#############################################################
$(IRDA_UTILS_DIR1)/.source: $(DL_DIR)/$(IRDA_UTILS_SOURCE)
	zcat $(DL_DIR)/$(IRDA_UTILS_SOURCE) | tar -C $(BUILD_DIR) -xvf -
	touch $(IRDA_UTILS_DIR1)/.source

$(IRDA_UTILS_DIR1)/.configured: $(IRDA_UTILS_DIR1)/.source
	touch $(IRDA_UTILS_DIR1)/.configured;

$(IRDA_UTILS_DIR1)/$(IRDA_UTILS_BINARY): $(IRDA_UTILS_DIR1)/.configured
	$(MAKE) SYS_INCLUDES= SYS_LIBPATH= CC=$(TARGET_CC) -C $(IRDA_UTILS_DIR1)/irattach

$(TARGET_DIR)/$(IRDA_UTILS_TARGET_BINARY): $(IRDA_UTILS_DIR1)/$(IRDA_UTILS_BINARY)
	STRIPPROG='$(STRIPPROG)' \
	$(MAKE) ROOT=$(TARGET_DIR) -C $(IRDA_UTILS_DIR1)/irattach install
	rm -Rf $(TARGET_DIR)/usr/man
	cp package/irda-utils/irda.rc $(TARGET_DIR)/etc/init.d/S50irda
	chmod 755 $(TARGET_DIR)/etc/init.d/S50irda
	mkdir -p $(TARGET_DIR)/etc/sysconfig
	cp package/irda-utils/irda.sysconfig $(TARGET_DIR)/etc/sysconfig/irda

irda-utils: uclibc $(TARGET_DIR)/$(IRDA_UTILS_TARGET_BINARY)

irda-utils-source: $(DL_DIR)/$(IRDA_UTILS_SOURCE)

irda-utils-clean:
	-$(MAKE) -C $(IRDA_UTILS_DIR1)/irattach clean

irda-utils-dirclean:
	rm -rf $(IRDA_UTILS_DIR1)


#############################################################
#
## Toplevel Makefile options
#
##############################################################
ifeq ($(strip $(BR2_PACKAGE_IRDA_UTILS)),y)
TARGETS+=irda-utils
endif
