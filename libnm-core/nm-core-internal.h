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
 * (C) Copyright 2014 Red Hat, Inc.
 */

#ifndef NM_CORE_NM_INTERNAL_H
#define NM_CORE_NM_INTERNAL_H

/* This header file contain functions that are provided as private API
 * by libnm-core. It will contain functions to give privileged access to
 * libnm-core. This can be useful for NetworkManager and libnm.so
 * which both are special users of libnm-core.
 * It also exposes some utility functions for reuse.
 *
 * These functions are not exported and are only available to components that link
 * statically against libnm-core. This basically means libnm-core, libnm, NetworkManager
 * and some test programs.
 **/

#include <glib.h>

#include "nm-utils.h"

#include "nm-setting-ip4-config.h"

const char *_nm_setting_ip4_config_get_address_label      (NMSettingIP4Config *setting,
                                                           guint32             i);
gboolean    _nm_setting_ip4_config_add_address_with_label (NMSettingIP4Config *setting,
                                                           NMIP4Address       *address,
                                                           const char         *label);

/* NM_SETTING_COMPARE_FLAG_INFERRABLE: check whether a device-generated
 * connection can be replaced by a already-defined connection. This flag only
 * takes into account properties marked with the %NM_SETTING_PARAM_INFERRABLE
 * flag.
 */
#define NM_SETTING_COMPARE_FLAG_INFERRABLE 0x80000000



#define NM_SETTING_SECRET_FLAGS_ALL \
	(NM_SETTING_SECRET_FLAG_NONE | \
	 NM_SETTING_SECRET_FLAG_AGENT_OWNED | \
	 NM_SETTING_SECRET_FLAG_NOT_SAVED | \
	 NM_SETTING_SECRET_FLAG_NOT_REQUIRED)


#endif
