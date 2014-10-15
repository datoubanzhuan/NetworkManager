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
 * Copyright 2004 - 2014 Red Hat, Inc.
 */

#include <string.h>
#include <gio/gio.h>

#include "nm-errors.h"
#include "nm-glib-compat.h"
#include "nm-dbus-interface.h"
#include "nm-core-internal.h"

G_DEFINE_QUARK (nm-connection-error-quark, nm_connection_error)
G_DEFINE_QUARK (nm-crypto-error-quark, nm_crypto_error)

static void
register_error_domain (GQuark domain,
                       const char *interface,
                       GType enum_type)
{
	GEnumClass *enum_class;
	GEnumValue *e;
	char *error_name;
	int i;

	enum_class = g_type_class_ref (enum_type);
	for (i = 0; i < enum_class->n_values; i++) {
		e = &enum_class->values[i];
		g_assert (strchr (e->value_nick, '-') == NULL);
		error_name = g_strdup_printf ("%s.%s", interface, e->value_nick);
		g_dbus_error_register_error (domain, e->value, error_name);
		g_free (error_name);
	}

	g_type_class_unref (enum_class);
}

void
_nm_dbus_errors_init (void)
{
	register_error_domain (NM_CONNECTION_ERROR,
	                       NM_DBUS_INTERFACE_SETTINGS_CONNECTION,
	                       NM_TYPE_CONNECTION_ERROR);
}
