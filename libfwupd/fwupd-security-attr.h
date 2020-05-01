/*
 * Copyright (C) 2020 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <glib-object.h>

#include "fwupd-enums.h"

G_BEGIN_DECLS

#define FWUPD_TYPE_SECURITY_ATTR (fwupd_security_attr_get_type ())
G_DECLARE_DERIVABLE_TYPE (FwupdSecurityAttr, fwupd_security_attr, FWUPD, SECURITY_ATTR, GObject)

struct _FwupdSecurityAttrClass
{
	GObjectClass			 parent_class;
	/*< private >*/
	void (*_fwupd_reserved1)	(void);
	void (*_fwupd_reserved2)	(void);
	void (*_fwupd_reserved3)	(void);
	void (*_fwupd_reserved4)	(void);
	void (*_fwupd_reserved5)	(void);
	void (*_fwupd_reserved6)	(void);
	void (*_fwupd_reserved7)	(void);
};


/**
 * FwupdSecurityAttrFlags:
 * @FWUPD_SECURITY_ATTR_FLAG_NONE:			No flags set
 * @FWUPD_SECURITY_ATTR_FLAG_SUCCESS:			Success
 * @FWUPD_SECURITY_ATTR_FLAG_OBSOLETED:			Obsoleted by another attribute
 * @FWUPD_SECURITY_ATTR_FLAG_RUNTIME_UPDATES:		Suffix `U`
 * @FWUPD_SECURITY_ATTR_FLAG_RUNTIME_ATTESTATION:	Suffix `A`
 * @FWUPD_SECURITY_ATTR_FLAG_RUNTIME_ISSUE:		Suffix `!`
 * @FWUPD_SECURITY_ATTR_FLAG_RUNTIME_UNTRUSTED:		Suffix `?`
 *
 * The flags available for HSI attributes.
 **/
typedef enum {
	FWUPD_SECURITY_ATTR_FLAG_NONE			= 0,
	FWUPD_SECURITY_ATTR_FLAG_SUCCESS		= 1 << 0,
	FWUPD_SECURITY_ATTR_FLAG_OBSOLETED		= 1 << 1,
	FWUPD_SECURITY_ATTR_FLAG_RUNTIME_UPDATES	= 1 << 8,
	FWUPD_SECURITY_ATTR_FLAG_RUNTIME_ATTESTATION	= 1 << 9,
	FWUPD_SECURITY_ATTR_FLAG_RUNTIME_ISSUE		= 1 << 10,
	FWUPD_SECURITY_ATTR_FLAG_RUNTIME_UNTRUSTED	= 1 << 11,
} FwupdSecurityAttrFlags;

/* the maximum value defined, although this might be increased in the future */
#define FWUPD_SECURITY_ATTR_NUMBER_MAX			5

FwupdSecurityAttr *fwupd_security_attr_new		(const gchar		*appstream_id);
gchar		*fwupd_security_attr_to_string		(FwupdSecurityAttr	*self);

const gchar	*fwupd_security_attr_get_appstream_id	(FwupdSecurityAttr	*self);
void		 fwupd_security_attr_set_appstream_id	(FwupdSecurityAttr	*self,
							 const gchar		*appstream_id);
guint32		 fwupd_security_attr_get_number		(FwupdSecurityAttr	*self);
void		 fwupd_security_attr_set_number		(FwupdSecurityAttr	*self,
							 guint32		 number);
const gchar	*fwupd_security_attr_get_name		(FwupdSecurityAttr	*self);
void		 fwupd_security_attr_set_name		(FwupdSecurityAttr	*self,
							 const gchar		*name);
const gchar	*fwupd_security_attr_get_summary	(FwupdSecurityAttr	*self);
void		 fwupd_security_attr_set_summary	(FwupdSecurityAttr	*self,
							 const gchar		*summary);
GPtrArray	*fwupd_security_attr_get_obsoletes	(FwupdSecurityAttr	*self);
void		 fwupd_security_attr_add_obsolete	(FwupdSecurityAttr	*self,
							 const gchar		*appstream_id);
gboolean	 fwupd_security_attr_has_obsolete	(FwupdSecurityAttr	*self,
							 const gchar		*appstream_id);
FwupdSecurityAttrFlags fwupd_security_attr_get_flags	(FwupdSecurityAttr	*self);
void		 fwupd_security_attr_set_flags		(FwupdSecurityAttr	*self,
							 FwupdSecurityAttrFlags flags);
void		 fwupd_security_attr_add_flag		(FwupdSecurityAttr	*self,
							 FwupdSecurityAttrFlags flag);
gboolean	 fwupd_security_attr_has_flag		(FwupdSecurityAttr	*self,
							 FwupdSecurityAttrFlags flag);
const gchar	*fwupd_security_attr_flag_to_string	(FwupdSecurityAttrFlags flag);
const gchar	*fwupd_security_attr_flag_to_suffix	(FwupdSecurityAttrFlags flag);

FwupdSecurityAttr *fwupd_security_attr_from_variant	(GVariant		*value);
GPtrArray	*fwupd_security_attr_array_from_variant	(GVariant		*value);

G_END_DECLS
