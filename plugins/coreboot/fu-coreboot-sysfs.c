/*
 * Copyright (C) 2019 9elements Agency GmbH <patrick.rudolph@9elements.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include <string.h>
#include <stdio.h>

#include "fu-plugin-coreboot.h"

/* detect the 'coreboot' kernel module presence. */
gboolean
fu_plugin_coreboot_sysfs_probe (void)
{
	return g_file_test (COREBOOT_SYSFS_FN, G_FILE_TEST_EXISTS| G_FILE_TEST_IS_DIR);
}

/* iterates over sysfs directories until the given ID is found. */
static gchar *
fu_plugin_coreboot_find_sysfs (const guint id,
			       const gchar *base_path,
			       const gchar *extension_path,
			       GError **error)
{
	const gchar *subdir = NULL;
	g_autoptr(GDir) dir = NULL;

	dir = g_dir_open (base_path, 0, error);
	if (dir == NULL)
		return NULL;
	while ((subdir = g_dir_read_name(dir)) != NULL) {
		guint64 i;
		g_autofree gchar *fp = NULL;
		g_autofree gchar *buf = NULL;
		fp = g_build_filename (base_path, subdir, extension_path, "id", NULL);
		if (!g_file_get_contents (fp, &buf, NULL, NULL))
			continue;
		i = g_ascii_strtoull (buf, NULL, 16);
		if (id == i)
			return g_build_filename (base_path, subdir, extension_path, NULL);
	}
	g_set_error (error,
		     G_IO_ERROR,
		     G_IO_ERROR_INVALID_ARGUMENT,
		     "Requested id not found");
	return NULL;
}

/* returns the coreboot tables with given tag */
gchar *
fu_plugin_coreboot_find_cb_table (const guint tag, gsize *length, GError **error)
{
	g_autofree gchar *fp = NULL;
	g_autofree gchar *path = NULL;
	gchar *buf;

	path = fu_plugin_coreboot_find_sysfs (tag, COREBOOT_SYSFS_FN "/devices/",
					      "attributes", error);
	if (path == NULL)
		return NULL;
	fp = g_build_filename (path, "data", NULL);
	if (!g_file_get_contents (fp, &buf, length, error))
		return NULL;
	return buf;
}

/* returns the CBMEM buffer with given id */
gchar *
fu_plugin_coreboot_find_cbmem (const guint id, gsize *length, goffset *address, GError **error)
{
	gchar *buf;
	g_autofree gchar *fp = NULL;
	g_autofree gchar *path = NULL;

	path = fu_plugin_coreboot_find_sysfs (id, COREBOOT_SYSFS_FN "/drivers/cbmem/",
					      "cbmem_attributes", error);
	if (path == NULL)
		return NULL;
	if (address != NULL) {
		g_autofree gchar *fp_tmp = NULL;
		g_autofree gchar *buf_tmp = NULL;
		fp_tmp = g_build_filename (path, "address", NULL);
		if (!g_file_get_contents (fp_tmp, &buf_tmp, NULL, error))
			return NULL;
		/* store the physical CBMEM buffer address if requested */
		*address = (goffset) g_ascii_strtoull (buf_tmp, NULL, 16);
	}
	fp = g_build_filename (path, "data", NULL);
	if (!g_file_get_contents (fp, &buf, length, error))
		return NULL;
	return buf;
}
