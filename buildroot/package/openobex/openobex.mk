#############################################################
#
# openobex
#
#############################################################

OPENOBEX_VER=1.2
OPENOBEX_DIR=$(BUILD_DIR)/openobex-$(OPENOBEX_VER)
OPENOBEX_SOURCE=openobex-$(OPENOBEX_VER).tar.gz
OPENOBEX_SITE:=http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/sourceforge/openobex
OPENOBEX_BINARY:=ircp/ircp
OPENOBEX_LIB:=lib/.libs/libopenobex.so
OPENOBEX_TARGET_LIB:=usr/lib/libopenobex.so
OPENOBEX_TARGET_BINARY:=usr/sbin/ircp

STRIPPROG=$(STRIP)

$(DL_DIR)/$(OPENOBEX_SOURCE):
	$(WGET) -P $(DL_DIR) $(OPENOBEX_SITE)/$(OPENOBEX_SOURCE)

#############################################################
#
# build modutils for use on the target system
#
#############################################################
$(OPENOBEX_DIR)/.source: $(DL_DIR)/$(OPENOBEX_SOURCE)
	zcat $(DL_DIR)/$(OPENOBEX_SOURCE) | tar -C $(BUILD_DIR) -xvf -
	touch $(OPENOBEX_DIR)/.source

$(OPENOBEX_DIR)/.configured: $(OPENOBEX_DIR)/.source
	(cd $(OPENOBEX_DIR); rm -rf config.cache; \
		$(TARGET_CONFIGURE_OPTS) \
		./configure --cache=/dev/null \
		--host=$(REAL_GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--prefix=/usr \
		--exec-prefix=/usr \
		--bindir=/usr/bin \
		--sbindir=/usr/sbin \
		--libexecdir=/usr/lib \
		--sysconfdir=/etc \
		--datadir=/usr/share \
		--localstatedir=/var \
		--mandir=/usr/share/man \
		--infodir=/usr/share/info \
		$(DISABLE_NLS) \
		--enable-shared \
		--enable-apps \
	);
	touch $(OPENOBEX_DIR)/.configured

$(OPENOBEX_DIR)/$(OPENOBEX_BINARY): $(OPENOBEX_DIR)/.configured
	$(MAKE) $(JLEVEL) -C $(OPENOBEX_DIR)/lib
	$(MAKE) $(JLEVEL) -C $(OPENOBEX_DIR)/ircp

$(TARGET_DIR)/$(OPENOBEX_TARGET_BINARY): $(OPENOBEX_DIR)/$(OPENOBEX_BINARY)
	$(MAKE) DESTDIR=$(TARGET_DIR) -C $(OPENOBEX_DIR)/lib install
	$(MAKE) DESTDIR=$(TARGET_DIR) -C $(OPENOBEX_DIR)/ircp install
	rm -rf $(TARGET_DIR)/share/locale $(TARGET_DIR)/usr/info \
		$(TARGET_DIR)/usr/man $(TARGET_DIR)/usr/share/doc \
		$(STAGING_DIR)/include/openobex
	-mv $(TARGET_DIR)/usr/include/openobex $(STAGING_DIR)/include
	

openobex: uclibc $(TARGET_DIR)/$(OPENOBEX_TARGET_BINARY)

openobex-source: $(DL_DIR)/$(OPENOBEX_SOURCE)

openobex-clean:
	-$(MAKE) -C $(OPENOBEX_DIR) clean

openobex-dirclean:
	rm -rf $(OPENOBEX_DIR)


#############################################################
#
## Toplevel Makefile options
#
##############################################################
ifeq ($(strip $(BR2_PACKAGE_OPENOBEX)),y)
TARGETS+=openobex
endif
