/* SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only */

#ifndef FLASHMAP_SERIALIZED_H__
#define FLASHMAP_SERIALIZED_H__

#include <stdint.h>

#define FMAP_SIGNATURE          "__FMAP__"
#define FMAP_VER_MAJOR          1       /* this header's FMAP minor version */
#define FMAP_VER_MINOR          1       /* this header's FMAP minor version */
#define FMAP_STRLEN             32      /* maximum length for strings, */
                                        /* including null-terminator */

enum fmap_flags {
        FMAP_AREA_STATIC        = 1 << 0,
        FMAP_AREA_COMPRESSED    = 1 << 1,
        FMAP_AREA_RO            = 1 << 2,
        FMAP_AREA_PRESERVE      = 1 << 3,
};

/* Mapping of volatile and static regions in firmware binary */
struct fmap_area {
        guint32 offset;                /* offset relative to base */
        guint32 size;                  /* size in bytes */
        guint8  name[FMAP_STRLEN];     /* descriptive name */
        guint16 flags;                 /* flags for this area */
}  __attribute__((__packed__));

struct fmap {
        guint8  signature[8];          /* "__FMAP__" (0x5F5F464D41505F5F) */
        guint8  ver_major;             /* major version */
        guint8  ver_minor;             /* minor version */
        guint64 base;                  /* address of the firmware binary */
        guint32 size;                  /* size of firmware binary in bytes */
        guint8  name[FMAP_STRLEN];     /* name of this firmware binary */
        guint16 nareas;                /* number of areas described by
                                           fmap_areas[] below */
        struct fmap_area areas[];
} __attribute__((__packed__));

#endif  /* FLASHMAP_SERIALIZED_H__ */

