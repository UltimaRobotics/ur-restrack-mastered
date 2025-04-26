include $(TOPDIR)/rules.mk

PKG_NAME:=ur-netresource
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/ur-netressource
  SECTION:=net
  CATEGORY:=Network
  TITLE:=ur-netresource
  DEPENDS:=
  MAINTAINER:=Fehmi Yousfi <fehmi_yousfi@hotmail.com>
endef

define Package/ur-netressource/description
	Advanced network status handler Designed by Ultima Robotics for system monitoring
	Supports multiple network interfaces
	Supports multiple network protocols	
endef

define Build/Configure
    # Empty body to skip the configure step
endef

define Build/Prepare
	if [ ! -d $(PKG_BUILD_DIR) ]; then \
		mkdir -p $(PKG_BUILD_DIR); \
	elif [ -n "$(ls -A $(PKG_BUILD_DIR))" ]; then \
		rm -r $(PKG_BUILD_DIR); \
		mkdir -p $(PKG_BUILD_DIR); \
	else \
		rm -r $(PKG_BUILD_DIR); \
		mkdir -p $(PKG_BUILD_DIR); \
	fi
	$(CP) ./pkg_src* $(PKG_BUILD_DIR)
endef

define Build/Compile
	echo "Building $(PKG_NAME) $(PKG_VERSION) in $(PKG_BUILD_DIR)"
	$(MAKE) -C $(PKG_BUILD_DIR)/pkg_src CC=$(TARGET_CC) 
	cp $(PKG_BUILD_DIR)/pkg_src/build/github_updater $(PKG_BUILD_DIR)/$(PKG_NAME)
endef

define Package/ur-netressource/install
	$(INSTALL_DIR) $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/bin/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ur-netressource.init $(1)/etc/init.d/ur-netressource

	$(INSTALL_DIR) $(1)/log/ultima-process
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/pkg_src/github_updater.log $(1)/log/ultima-process/ultima_robotics_updater.log
	$(INSTALL_DIR) $(1)/etc/updater-config
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/pkg_src/github_updater_config.json $(1)/etc/updater-config/ultima_robotics_updater_config.json

endef

define Build/clean
	rm -rf $(PKG_BUILD_DIR)
endef

$(eval $(call BuildPackage,ur-netressource))
