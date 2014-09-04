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
 * Copyright 2007 - 2008 Novell, Inc.
 * Copyright 2008 - 2014 Red Hat, Inc.
 */

#ifndef __NM_IP6_CONFIG_H__
#define __NM_IP6_CONFIG_H__

#if !defined (__NETWORKMANAGER_H_INSIDE__) && !defined (NETWORKMANAGER_COMPILATION)
#error "Only <NetworkManager.h> can be included directly."
#endif

#include <glib.h>
#include <glib-object.h>
#include "nm-object.h"

G_BEGIN_DECLS

#define NM_TYPE_IP6_CONFIG            (nm_ip6_config_get_type ())
#define NM_IP6_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NM_TYPE_IP6_CONFIG, NMIP6Config))
#define NM_IP6_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NM_TYPE_IP6_CONFIG, NMIP6ConfigClass))
#define NM_IS_IP6_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NM_TYPE_IP6_CONFIG))
#define NM_IS_IP6_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), NM_TYPE_IP6_CONFIG))
#define NM_IP6_CONFIG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), NM_TYPE_IP6_CONFIG, NMIP6ConfigClass))

typedef struct {
	NMObject parent;
} NMIP6Config;

typedef struct {
	NMObjectClass parent;

	/*< private >*/
	gpointer padding[4];
} NMIP6ConfigClass;

#define NM_IP6_CONFIG_GATEWAY "gateway"
#define NM_IP6_CONFIG_ADDRESSES "addresses"
#define NM_IP6_CONFIG_ROUTES "routes"
#define NM_IP6_CONFIG_NAMESERVERS "nameservers"
#define NM_IP6_CONFIG_DOMAINS "domains"
#define NM_IP6_CONFIG_SEARCHES "searches"

GType nm_ip6_config_get_type (void);

const char *           nm_ip6_config_get_gateway         (NMIP6Config *config);
const GSList *         nm_ip6_config_get_addresses       (NMIP6Config *config);
const GSList *         nm_ip6_config_get_routes          (NMIP6Config *config);
guint32                nm_ip6_config_get_num_nameservers (NMIP6Config *config);
const struct in6_addr *nm_ip6_config_get_nameserver      (NMIP6Config *config, guint32 idx);
const GSList *         nm_ip6_config_get_nameservers     (NMIP6Config *config);
const char * const *   nm_ip6_config_get_domains         (NMIP6Config *config);
const char * const *   nm_ip6_config_get_searches        (NMIP6Config *config);

G_END_DECLS

#endif /* __NM_IP6_CONFIG_H__ */
