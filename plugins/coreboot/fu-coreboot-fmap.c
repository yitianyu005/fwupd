/*
 * Copyright (C) 2019 9elements Agency GmbH <patrick.rudolph@9elements.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include <string.h>
#include <stdio.h>

#include "fu-plugin-coreboot.h"
#include "fu-coreboot-cbmem.h"
#include "fu-coreboot-fmap.h"

/* Returns the FMAP holding the firmware flash layout. */
const struct fmap *
fu_plugin_coreboot_get_fmap (GError **error)
{
	const struct fmap *hdr;
	gsize len;

	hdr = (const struct fmap *) fu_plugin_coreboot_find_cbmem (CBMEM_ID_FMAP,
								   &len,
								   NULL,
								   error);
	if (len < sizeof (*hdr)) {
		g_free (hdr);
		return NULL;
	}

	/* Sanity check. */
	if (memcmp (hdr->signature, FMAP_SIGNATURE, sizeof(hdr->signature)) ||
	    hdr->ver_major != FMAP_VER_MAJOR ||
	    hdr->nareas == 0 ||
	    hdr->size == 0 ||
	    len < sizeof(*hdr) + hdr->nareas * sizeof(struct fmap_area)) {
		g_free (hdr);
		hdr = NULL;
	}

	return hdr;
}

/*
 * Sets the first matching FMAP region identified by name in out.
 * out can be NULL.
 * Returns True on success and False on error.
 */
gboolean
fu_plugin_coreboot_fmap_region_by_name (struct fmap *fmd,
					const gchar *name,
					struct fmap_area **out,
					GError **error)
{
	for (gsize i = 0; i < fmd->nareas; i++) {
		if (!g_strcmp0 (name, (const gchar *)fmd->areas[i].name)) {
			if (out)
				*out = &fmd->areas[i];
			return TRUE;
		}
	}
	return FALSE;
}