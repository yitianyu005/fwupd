/*
 * Copyright (C) 2020 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "fu-common.h"
#include "fu-uefi-dbx-common.h"

static gint
fu_common_filename_glob_sort_cb (gconstpointer a, gconstpointer b)
{
	return g_strcmp0 (*(const gchar **)a, *(const gchar **)b);
}

static GPtrArray *
fu_common_filename_glob (const gchar *directory, const gchar *pattern, GError **error)
{
	const gchar *basename;
	g_autoptr(GDir) dir = g_dir_open (directory, 0, error);
	g_autoptr(GPtrArray) files = g_ptr_array_new_with_free_func (g_free);
	if (dir == NULL)
		return NULL;
	while ((basename = g_dir_read_name (dir)) != NULL) {
		if (!fu_common_fnmatch (pattern, basename))
			continue;
		g_ptr_array_add (files, g_build_filename (directory, basename, NULL));
	}
	if (files->len == 0) {
		g_set_error_literal (error,
				     G_IO_ERROR,
				     G_IO_ERROR_NOT_FOUND,
				     "no files matched pattern");
		return NULL;
	}
	g_ptr_array_sort (files, fu_common_filename_glob_sort_cb);
	return g_steal_pointer (&files);
}

gchar *
fu_uefi_dbx_get_dbxupdate (GError **error)
{
	g_autofree gchar *dbxdir = NULL;
	g_autofree gchar *glob = NULL;
	g_autoptr(GPtrArray) files = NULL;

	/* fall back out of prefix */
	dbxdir = g_build_filename (FWUPD_DATADIR, "dbxtool", NULL);
	if (!g_file_test (dbxdir, G_FILE_TEST_EXISTS)) {
		g_free (dbxdir);
		dbxdir = g_strdup ("/usr/share/dbxtool");
	}
	if (!g_file_test (dbxdir, G_FILE_TEST_EXISTS)) {
		g_free (dbxdir);
		dbxdir = g_strdup ("/usr/share/secureboot/updates/dbx");
	}

	/* get the newest files from dbxtool, prefer the per-arch ones first */
	glob = g_strdup_printf ("*%s*.bin", EFI_MACHINE_TYPE_NAME);
	files = fu_common_filename_glob (dbxdir, glob, NULL);
	if (files == NULL)
		files = fu_common_filename_glob (dbxdir, "*.bin", error);
	if (files == NULL)
		return NULL;
	return g_strdup (g_ptr_array_index (files, 0));
}
