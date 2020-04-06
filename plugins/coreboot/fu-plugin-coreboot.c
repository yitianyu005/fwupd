/*
 * Copyright (C) 2019 9elements Agency GmbH <patrick.rudolph@9elements.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include <string.h>
#include <stdio.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "fu-plugin-vfuncs.h"
#include "fu-hash.h"
#include "fu-device-metadata.h"
#include "fu-device-private.h"
#include "fu-plugin-coreboot.h"
#include "fu-coreboot-tables.h"

void
fu_plugin_init (FuPlugin *plugin)
{
	fu_plugin_set_build_hash (plugin, FU_BUILD_HASH);
}

static void
fu_plugin_debuginfos (FuPlugin *plugin)
{
	g_autofree const struct lb_boot_media_params *bmp = NULL;
	g_autoptr(GError) error = NULL;

	if (!fu_plugin_coreboot_sysfs_probe ())
		return;

	/* The following are supported since Linux 5.6 */
	bmp = fu_plugin_coreboot_get_bootmedia_params (&error);
	if (error)
		g_debug ("Could not read boot media params: %s", error->message);
	if (bmp) {
		g_debug ("BMP: Active CBFS at offset 0x%08lx", bmp->cbfs_offset);
		g_debug ("BMP: Active CBFS size 0x%08lx", bmp->cbfs_size);
		g_free (bmp);
	}
}

gboolean
fu_plugin_coldplug (FuPlugin *plugin, GError **error)
{
	const gchar *major;
	const gchar *minor;
	const gchar *version;
	GBytes *bios_table;
	gboolean updatable = FALSE; /* TODO: Implement update support */
	g_autofree gchar *name = NULL;
	g_autofree gchar *triplet = NULL;
	g_autoptr(FuDevice) dev = NULL;
	gboolean has_vboot = FALSE;

	/* don't include FU_HWIDS_KEY_BIOS_VERSION */
	static const gchar *hwids[] = {
		"HardwareID-3",
		"HardwareID-4",
		"HardwareID-5",
		"HardwareID-6",
		"HardwareID-10",
	};

	fu_plugin_debuginfos (plugin);

	version = fu_plugin_coreboot_get_version_string (plugin);
	if (version != NULL)
		triplet = fu_plugin_coreboot_version_string_to_triplet (version, error);

	if (version == NULL || triplet == NULL) {
		major = fu_plugin_get_dmi_value (plugin, FU_HWIDS_KEY_BIOS_MAJOR_RELEASE);
		if (major == NULL) {
			g_set_error (error,
				     G_IO_ERROR,
				     G_IO_ERROR_NOT_FOUND,
				     "Missing BIOS major release");
			return FALSE;
		}
		minor = fu_plugin_get_dmi_value (plugin, FU_HWIDS_KEY_BIOS_MINOR_RELEASE);
		if (minor == NULL) {
			g_set_error (error,
				     G_IO_ERROR,
				     G_IO_ERROR_NOT_FOUND,
				     "Missing BIOS minor release");
			return FALSE;
		}
		triplet = g_strdup_printf ("%s.%s.0", major, minor);
	}

	if (triplet == NULL) {
		g_set_error (error,
			     G_IO_ERROR,
			     G_IO_ERROR_INVALID_DATA,
			     "No version string found");

		return FALSE;
	}
	dev = fu_device_new ();
	fu_device_set_version_format (dev, FWUPD_VERSION_FORMAT_TRIPLET);
	fu_device_set_version (dev, triplet);
	fu_device_set_summary (dev, "Open Source system boot firmware");
	fu_device_set_id (dev, "coreboot");
	fu_device_add_flag (dev, FWUPD_DEVICE_FLAG_INTERNAL);
	fu_device_add_icon (dev, "computer");
	name = fu_plugin_coreboot_get_name_for_type (plugin, NULL);
	if (name != NULL) {
		fu_device_set_name (dev, name);
	} else {
		fu_device_set_name (dev, fu_plugin_get_dmi_value (plugin, FU_HWIDS_KEY_PRODUCT_NAME));
	}
	fu_device_set_vendor (dev, fu_plugin_get_dmi_value (plugin, FU_HWIDS_KEY_MANUFACTURER));
	fu_device_add_instance_id (dev, "main-system-firmware");
	fu_device_set_vendor_id (dev, "DMI:coreboot");

	for (guint i = 0; i < G_N_ELEMENTS (hwids); i++) {
		char *str;
		str = fu_plugin_get_hwid_replace_value (plugin, hwids[i], NULL);
		if (str != NULL)
			fu_device_add_instance_id (dev, str);
	}

	bios_table = fu_plugin_get_smbios_data (plugin, FU_SMBIOS_STRUCTURE_TYPE_BIOS);
	if (bios_table != NULL) {
		guint32 bios_characteristics;
		gsize len;
		const guint8 *value = g_bytes_get_data (bios_table, &len);
		if (len >= 0x9) {
			gint firmware_size = (value[0x9] + 1) * 64 * 1024;
			fu_device_set_firmware_size_max (dev, firmware_size);
		}
		if (len >= (0xa + sizeof(guint32))) {
			memcpy (&bios_characteristics, &value[0xa], sizeof (guint32));
			/* Read the "BIOS is upgradeable (Flash)" flag */
			if (!(bios_characteristics & (1 << 11)))
				updatable = FALSE;
		}
	}

	if (updatable)
		fu_device_add_flag (dev, FWUPD_DEVICE_FLAG_UPDATABLE);

	if (fu_plugin_coreboot_sysfs_probe()) {
		g_autoptr(GError) sysfs_error = NULL;
		has_vboot = fu_plugin_coreboot_has_vboot (&sysfs_error);
		if (sysfs_error)
			g_debug ("Could not get VBOOT enabledment status: %s", sysfs_error->message);
	}

	if (has_vboot)
		fu_device_add_flag (dev, FWUPD_DEVICE_FLAG_CAN_VERIFY_IMAGE);

	/* convert instances to GUID */
	fu_device_convert_instance_ids (dev);

	fu_plugin_device_add (plugin, dev);

	return TRUE;
}

gboolean
fu_plugin_startup (FuPlugin *plugin, GError **error)
{
	const gchar *vendor;

	vendor = fu_plugin_get_dmi_value (plugin, FU_HWIDS_KEY_BIOS_VENDOR);
	if (g_strcmp0 (vendor, "coreboot") != 0) {
		g_set_error (error,
			     FWUPD_ERROR,
			     FWUPD_ERROR_NOT_FOUND,
			     "No coreboot detected on this machine.");
		return FALSE;
	}

	return TRUE;
}

