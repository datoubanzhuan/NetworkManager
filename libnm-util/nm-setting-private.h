/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 * (C) Copyright 2011 Red Hat, Inc.
 */

#ifndef NM_SETTING_PRIVATE_H
#define NM_SETTING_PRIVATE_H

#include "nm-glib-compat.h"

#define NM_SETTING_SECRET_FLAGS_ALL \
	(NM_SETTING_SECRET_FLAG_NONE | \
	 NM_SETTING_SECRET_FLAG_AGENT_OWNED | \
	 NM_SETTING_SECRET_FLAG_NOT_SAVED | \
	 NM_SETTING_SECRET_FLAG_NOT_REQUIRED)

void _nm_register_setting (const char *name,
                           const GType type,
                           const guint32 priority,
                           const GQuark error_quark);
gboolean _nm_setting_is_base_type (NMSetting *setting);
GType _nm_setting_lookup_setting_type (const char *name);
GType _nm_setting_lookup_setting_type_by_quark (GQuark error_quark);
gint _nm_setting_compare_priority (gconstpointer a, gconstpointer b);

/* NM_SETTING_COMPARE_FLAG_CANDIDATE: check a whether a device-generated connection
 * can be meaningfully replaced by a configured connection. With this flag
 * the matching function is asymetric and only takes into account properties
 * mandated by the candidate configured connection. It is for internal use by
 * NetworkManager.
 */
#define NM_SETTING_COMPARE_FLAG_CANDIDATE 0x80000000

/* Ensure the setting's GType is registered at library load time */
#define NM_SETTING_REGISTER_TYPE(x) \
static void __attribute__((constructor)) register_setting (void) \
{ g_type_init (); g_type_ensure (x); }

NMSetting *nm_setting_find_in_list (GSList *settings_list, const char *setting_name);

#endif  /* NM_SETTING_PRIVATE_H */

