/*
 * Copyright (C) 2020 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "fu-plugin-vfuncs.h"
#include "fu-efivar.h"
#include "fu-hash.h"
#include "fu-uefi-dbx-common.h"
#include "fu-uefi-dbx-file.h"

struct FuPluginData {
	gchar			*fn;
};

void
fu_plugin_init (FuPlugin *plugin)
{
	fu_plugin_alloc_data (plugin, sizeof (FuPluginData));
	fu_plugin_set_build_hash (plugin, FU_BUILD_HASH);
}

void
fu_plugin_destroy (FuPlugin *plugin)
{
	FuPluginData *data = fu_plugin_get_data (plugin);
	g_free (data->fn);
}

gboolean
fu_plugin_startup (FuPlugin *plugin, GError **error)
{
	FuPluginData *data = fu_plugin_get_data (plugin);
	data->fn = fu_uefi_dbx_get_dbxupdate (error);
	if (data->fn == NULL)
		return FALSE;
	g_debug ("using %s", data->fn);
	return TRUE;
}

gboolean
fu_plugin_add_security_attrs (FuPlugin *plugin, GPtrArray *attrs, GError **error)
{
	FuPluginData *data = fu_plugin_get_data (plugin);
	FwupdSecurityAttr *attr;
	GPtrArray *checksums;
	gsize bufsz = 0;
	guint missing_cnt = 0;
	g_autofree guint8 *buf_system = NULL;
	g_autofree guint8 *buf_update = NULL;
	g_autoptr(FuUefiDbxFile) dbx_system = NULL;
	g_autoptr(FuUefiDbxFile) dbx_update = NULL;
	g_autoptr(GError) error_local = NULL;

	/* get update dbx */
	if (!g_file_get_contents (data->fn, (gchar **) &buf_update, &bufsz, error)) {
		g_prefix_error (error, "failed to load %s: ", data->fn);
		return FALSE;
	}
	dbx_update = fu_uefi_dbx_file_new (buf_update, bufsz,
					   FU_UEFI_DBX_FILE_PARSE_FLAGS_IGNORE_HEADER,
					   error);
	if (dbx_update == NULL) {
		g_prefix_error (error, "could not parse %s: ", data->fn);
		return FALSE;
	}

	/* get system dbx */
	if (!fu_efivar_get_data ("d719b2cb-3d3a-4596-a3bc-dad00e67656f", "dbx",
				 &buf_system, &bufsz, NULL, error)) {
		g_prefix_error (error, "failed to get dbx: ");
		return FALSE;
	}
	dbx_system = fu_uefi_dbx_file_new (buf_system, bufsz,
					   FU_UEFI_DBX_FILE_PARSE_FLAGS_NONE,
					   error);
	if (dbx_system == NULL) {
		g_prefix_error (error, "could not parse variable: ");
		return FALSE;
	}

	/* look for each checksum in the update in the system version */
	checksums = fu_uefi_dbx_file_get_checksums (dbx_update);
	for (guint i = 0; i < checksums->len; i++) {
		const gchar *checksum = g_ptr_array_index (checksums, i);
		if (!fu_uefi_dbx_file_has_checksum (dbx_system, checksum)) {
			g_debug ("%s missing from the system dbx", checksum);
			missing_cnt += 1;
		}
	}

	/* none configured */
	attr = fwupd_security_attr_new ("org.uefi.SecureBoot.dbx");
	fwupd_security_attr_set_number (attr, 1);
	fwupd_security_attr_set_name (attr, "UEFI dbx");
	g_ptr_array_add (attrs, attr);

	/* add security attribute */
	if (missing_cnt == 0) {
		fwupd_security_attr_add_flag (attr, FWUPD_SECURITY_ATTR_FLAG_SUCCESS);
	} else {
		g_autofree gchar *summary = g_strdup_printf ("%u hashes missing", missing_cnt);
		fwupd_security_attr_set_summary (attr, summary);
	}
	return TRUE;
}
