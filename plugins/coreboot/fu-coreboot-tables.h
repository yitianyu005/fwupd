/*
 * Copyright (C) 2019 9elements Agency GmbH <patrick.rudolph@9elements.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

// FIXME: Move into coreboot-devel package
enum {
	LB_TAG_BOOT_MEDIA_PARAMS        = 0x0030,
	LB_TAG_VBOOT_WORKBUF            = 0x0034,
};

struct lb_boot_media_params {
	guint32 tag;
	guint32 size;
	/* offsets are relative to start of boot media */
	guint64 fmap_offset;
	guint64 cbfs_offset;
	guint64 cbfs_size;
	guint64 boot_media_size;
};

