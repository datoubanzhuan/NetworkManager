/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright 2013 - 2015 Red Hat, Inc.
 */

/**
 * SECTION:nm-vpn-helpers
 * @short_description: VPN-related utilities
 *
 * Some functions should probably eventually move into libnm.
 */

#include "config.h"

#include <string.h>
#include <glib.h>
#include <gmodule.h>

#include <NetworkManager.h>
#include "nm-vpn-editor-plugin.h"

#include "nm-vpn-helpers.h"


#define VPN_NAME_FILES_DIR NMCONFDIR "/VPN"
#define DEFAULT_DIR_LIB    NMLIBDIR"/VPN"

static gboolean plugins_loaded = FALSE;
static GHashTable *plugins_hash = NULL;
static GSList *plugins_list = NULL;


GQuark nm_vpn_error_quark (void);
G_DEFINE_QUARK (NM_VPN_ERROR, nm_vpn_error)
#define NM_VPN_ERROR nm_vpn_error_quark ()
#define NM_VPN_ERROR_GENERIC 0

NMVpnEditorPlugin *
nm_vpn_get_plugin_by_service (const char *service)
{
	NMVpnEditorPlugin *plugin;
	const char *str;
	char *tmp = NULL;

	g_return_val_if_fail (service != NULL, NULL);

	if (G_UNLIKELY (!plugins_loaded))
		nm_vpn_get_plugins (NULL);

	if (!plugins_hash)
		return NULL;

	if (g_str_has_prefix (service, NM_DBUS_SERVICE))
		str = service;
	else
		str = tmp = g_strdup_printf ("%s.%s", NM_DBUS_SERVICE, service);

	plugin = g_hash_table_lookup (plugins_hash, str);
	g_free (tmp);

	return plugin;
}

GSList *
nm_vpn_get_plugins (GError **error)
{
	GDir *dir;
	const char *f;
	GHashTableIter iter;
	NMVpnEditorPlugin *plugin;

	if (error)
		g_return_val_if_fail (*error == NULL, NULL);

	if (G_LIKELY (plugins_loaded))
		return plugins_list;

	plugins_loaded = TRUE;

	dir = g_dir_open (VPN_NAME_FILES_DIR, 0, NULL);
	if (!dir) {
		g_set_error (error, NM_VPN_ERROR, NM_VPN_ERROR_GENERIC, "Couldn't read VPN .name files directory " VPN_NAME_FILES_DIR ".");
		return NULL;
	}

	plugins_hash = g_hash_table_new_full (g_str_hash, g_str_equal,
	                                      (GDestroyNotify) g_free, (GDestroyNotify) g_object_unref);

	while ((f = g_dir_read_name (dir))) {
		char *path = NULL, *service = NULL;
		char *so_path = NULL, *so_name = NULL;
		GKeyFile *keyfile = NULL;
		GModule *module = NULL;
		NMVpnEditorPluginFactory factory = NULL;

		if (!g_str_has_suffix (f, ".name"))
			continue;

		path = g_strdup_printf ("%s/%s", VPN_NAME_FILES_DIR, f);

		keyfile = g_key_file_new ();
		if (!g_key_file_load_from_file (keyfile, path, 0, NULL))
			goto next;

		service = g_key_file_get_string (keyfile, "VPN Connection", "service", NULL);
		if (!service)
			goto next;

		so_path = g_key_file_get_string (keyfile,  "libnm", "plugin", NULL);
		if (!so_path)
			goto next;

		if (g_path_is_absolute (so_path))
			module = g_module_open (so_path, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);

		if (!module) {
			/* Remove any path and extension components, then reconstruct path
			 * to the SO in LIBDIR
			 */
			so_name = g_path_get_basename (so_path);
			g_free (so_path);
			so_path = g_strdup_printf ("%s/NetworkManager/%s", NMLIBDIR, so_name);
			g_free (so_name);

			module = g_module_open (so_path, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
			if (!module) {
				g_clear_error (error);
				g_set_error (error, NM_VPN_ERROR, NM_VPN_ERROR_GENERIC, "Cannot load the VPN plugin which provides the "
				             "service '%s'.", service);
				goto next;
			}
		}

		if (g_module_symbol (module, "nm_vpn_editor_plugin_factory", (gpointer) &factory)) {
			GError *factory_error = NULL;
			gboolean success = FALSE;

			plugin = factory (&factory_error);
			if (plugin) {
				char *plug_name = NULL, *plug_service = NULL;

				/* Validate plugin properties */
				g_object_get (G_OBJECT (plugin),
				              NM_VPN_EDITOR_PLUGIN_NAME, &plug_name,
				              NM_VPN_EDITOR_PLUGIN_SERVICE, &plug_service,
				              NULL);
				if (!plug_name || !strlen (plug_name)) {
					g_clear_error (error);
					g_set_error (error, NM_VPN_ERROR, NM_VPN_ERROR_GENERIC, "cannot load VPN plugin in '%s': missing plugin name", 
					             g_module_name (module));
				} else if (!plug_service || strcmp (plug_service, service)) {
					g_clear_error (error);
					g_set_error (error, NM_VPN_ERROR, NM_VPN_ERROR_GENERIC, "cannot load VPN plugin in '%s': invalid service name", 
					             g_module_name (module));
				} else {
					/* Success! */
					g_object_set_data_full (G_OBJECT (plugin), "gmodule", module,
					                        (GDestroyNotify) g_module_close);
					g_hash_table_insert (plugins_hash, g_strdup (service), plugin);
					success = TRUE;
				}
				g_free (plug_name);
				g_free (plug_service);
			} else {
				g_clear_error (error);
				g_set_error (error, NM_VPN_ERROR, NM_VPN_ERROR_GENERIC, "cannot load VPN plugin in '%s': %s", 
				             g_module_name (module), g_module_error ());
			}

			if (!success)
				g_module_close (module);
		} else {
			g_clear_error (error);
			g_set_error (error, NM_VPN_ERROR, NM_VPN_ERROR_GENERIC, "cannot locate nm_vpn_editor_plugin_factory() in '%s': %s", 
			             g_module_name (module), g_module_error ());
			g_module_close (module);
		}

	next:
		g_free (so_path);
		g_free (service);
		g_key_file_free (keyfile);
		g_free (path);
	}
	g_dir_close (dir);

	/* Copy hash to list */
	g_hash_table_iter_init (&iter, plugins_hash);
	while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &plugin))
		plugins_list = g_slist_prepend (plugins_list, plugin);

	return plugins_list;
}

gboolean
nm_vpn_supports_ipv6 (NMConnection *connection)
{
	NMSettingVpn *s_vpn;
	const char *service_type;
	NMVpnEditorPlugin *plugin;
	guint32 capabilities;

	s_vpn = nm_connection_get_setting_vpn (connection);
	g_return_val_if_fail (s_vpn != NULL, FALSE);

	service_type = nm_setting_vpn_get_service_type (s_vpn);
	g_return_val_if_fail (service_type != NULL, FALSE);

	plugin = nm_vpn_get_plugin_by_service (service_type);
	g_return_val_if_fail (plugin != NULL, FALSE);

	capabilities = nm_vpn_editor_plugin_get_capabilities (plugin);
	return (capabilities & NM_VPN_EDITOR_PLUGIN_CAPABILITY_IPV6) != 0;
}