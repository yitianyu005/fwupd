/*
 * Copyright (C) 2020 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "fu-plugin-vfuncs.h"
#include "fu-hash.h"

#define FU_PLUGIN_SWAP_SOURCE_FILE	"/proc/swaps"

struct FuPluginData {
	GFileMonitor		*monitor;
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
	if (data->monitor != NULL)
		g_object_unref (data->monitor);
}

static void
fu_plugin_swap_changed_cb (GFileMonitor *monitor,
			   GFile *file,
			   GFile *other_file,
			   GFileMonitorEvent event_type,
			   gpointer user_data)
{
	FuPlugin *plugin = FU_PLUGIN (user_data);
	fu_plugin_security_changed (plugin);
}

gboolean
fu_plugin_startup (FuPlugin *plugin, GError **error)
{
	FuPluginData *data = fu_plugin_get_data (plugin);
	g_autoptr(GFile) file = g_file_new_for_path (FU_PLUGIN_SWAP_SOURCE_FILE);
	data->monitor = g_file_monitor (file, G_FILE_MONITOR_NONE, NULL, error);
	if (data->monitor == NULL)
		return FALSE;
	g_signal_connect (data->monitor, "changed",
			  G_CALLBACK (fu_plugin_swap_changed_cb), plugin);
	return TRUE;
}

gboolean
fu_plugin_add_security_attrs (FuPlugin *plugin, GPtrArray *attrs, GError **error)
{
	FwupdSecurityAttr *attr;
	gsize bufsz = 0;
	g_autofree gchar *buf = NULL;
	g_autoptr(GError) error_local = NULL;
	g_auto(GStrv) lines = NULL;
	gboolean swap_secure = TRUE;

	/* load list of swaps */
	if (!g_file_get_contents (FU_PLUGIN_SWAP_SOURCE_FILE, &buf, &bufsz, error)) {
		g_prefix_error (error, "could not open %s: ", FU_PLUGIN_SWAP_SOURCE_FILE);
		return FALSE;
	}

	/* none configured */
	attr = fwupd_security_attr_new ("org.kernel.Swap");
	fwupd_security_attr_add_flag (attr, FWUPD_SECURITY_ATTR_FLAG_RUNTIME_ISSUE);
	fwupd_security_attr_set_name (attr, "Linux Swap");
	g_ptr_array_add (attrs, attr);
	lines = fu_common_strnsplit (buf, bufsz, "\n", -1);
	if (g_strv_length (lines) <= 2) {
		fwupd_security_attr_add_flag (attr, FWUPD_SECURITY_ATTR_FLAG_SUCCESS);
		return TRUE;
	}

	/* check each swap, ignoring header line */
	for (guint i = 1; lines[i] != NULL && lines[i][0] != '\0'; i++) {
		g_warning ("lines[i]=%s", lines[i]);
		g_strdelimit (lines[i], " ", '\0');
		if (!g_str_has_prefix (lines[i], "/dev/dm-") &&
		    !g_str_has_prefix (lines[i], "/dev/mapper")) {
			swap_secure = FALSE;
			break;
		}
	}

	/* add security attribute */
	if (swap_secure) {
		fwupd_security_attr_add_flag (attr, FWUPD_SECURITY_ATTR_FLAG_SUCCESS);
		fwupd_security_attr_set_summary (attr, "Encrypted");
	} else {
		fwupd_security_attr_set_summary (attr, "Not encrypted");
	}
	return TRUE;
}
