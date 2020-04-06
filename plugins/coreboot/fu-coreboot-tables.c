/*
 * Copyright (C) 2019 9elements Agency GmbH <patrick.rudolph@9elements.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include <string.h>
#include <stdio.h>

#include "fu-plugin-coreboot.h"
#include "fu-coreboot-tables.h"

/* Returns the boot_media_params containing a pointer to the active CBFS. */
const struct lb_boot_media_params *
fu_plugin_coreboot_get_bootmedia_params (GError **error)
{
	const struct lb_boot_media_params *params;
	gsize len;
	
	params = (struct lb_boot_media_params *)
		fu_plugin_coreboot_find_cb_table (LB_TAG_BOOT_MEDIA_PARAMS,
						  &len,
						  error);
	if (len < sizeof (*params)) {
		g_free (params);
		params = NULL;
	}
	return params;
}

gboolean
fu_plugin_coreboot_has_vboot (GError **error)
{
	gboolean has_vboot;
	void *ptr;

	ptr = fu_plugin_coreboot_find_cb_table (LB_TAG_VBOOT_WORKBUF,
						NULL,
						error);
	has_vboot = ptr != NULL;
	g_free (ptr);
	return has_vboot;
}
