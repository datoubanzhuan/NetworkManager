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
 * Copyright 2008 Red Hat, Inc.
 */

#ifndef __NM_TYPES_H__
#define __NM_TYPES_H__

#if !defined (__NETWORKMANAGER_H_INSIDE__) && !defined (NETWORKMANAGER_COMPILATION)
#error "Only <NetworkManager.h> can be included directly."
#endif

#include <glib.h>
#include <glib-object.h>

#include <nm-enum-types.h>

G_BEGIN_DECLS

#define NM_TYPE_UINT_ARRAY  (nm_uint_array_get_type ())
GType     nm_uint_array_get_type (void) G_GNUC_CONST;

#define NM_TYPE_OBJECT_ARRAY  (nm_object_array_get_type ())
GType     nm_object_array_get_type (void) G_GNUC_CONST;

#define NM_TYPE_IP6_ADDRESS_OBJECT_ARRAY  (nm_ip6_address_object_array_get_type ())
GType     nm_ip6_address_object_array_get_type (void) G_GNUC_CONST;

#define NM_TYPE_IP6_ADDRESS_ARRAY  (nm_ip6_address_array_get_type ())
GType     nm_ip6_address_array_get_type (void) G_GNUC_CONST;

#define NM_TYPE_IP6_ROUTE_OBJECT_ARRAY  (nm_ip6_route_object_array_get_type ())
GType     nm_ip6_route_object_array_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __NM_TYPES_H__ */
