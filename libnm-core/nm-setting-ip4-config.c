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
 * Copyright 2014 Red Hat, Inc.
 */

#include "config.h"

#include <string.h>
#include <glib/gi18n-lib.h>

#include "nm-setting-ip4-config.h"
#include "nm-setting-private.h"

/**
 * SECTION:nm-setting-ip4-config
 * @short_description: Describes IPv4 addressing, routing, and name service properties
 *
 * The #NMSettingIP4Config object is a #NMSetting subclass that describes
 * properties related to IPv4 addressing, routing, and Domain Name Service.
 *
 * #NMSettingIP4Config has few properties or methods of its own; it inherits
 * almost everything from #NMSettingIPConfig.
 *
 * NetworkManager supports 5 values for the #NMSettingIPConfig:method property
 * for IPv4.  If "auto" is specified then the appropriate automatic method
 * (DHCP, PPP, etc) is used for the interface and most other properties can be
 * left unset.  If "link-local" is specified, then a link-local address in the
 * 169.254/16 range will be assigned to the interface.  If "manual" is
 * specified, static IP addressing is used and at least one IP address must be
 * given in the "addresses" property.  If "shared" is specified (indicating that
 * this connection will provide network access to other computers) then the
 * interface is assigned an address in the 10.42.x.1/24 range and a DHCP and
 * forwarding DNS server are started, and the interface is NAT-ed to the current
 * default network connection.  "disabled" means IPv4 will not be used on this
 * connection.
 **/

G_DEFINE_TYPE_WITH_CODE (NMSettingIP4Config, nm_setting_ip4_config, NM_TYPE_SETTING_IP_CONFIG,
                         _nm_register_setting (IP4_CONFIG, 4))
NM_SETTING_REGISTER_TYPE (NM_TYPE_SETTING_IP4_CONFIG)

#define NM_SETTING_IP4_CONFIG_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), NM_TYPE_SETTING_IP4_CONFIG, NMSettingIP4ConfigPrivate))

typedef struct {
	char *dhcp_client_id;
} NMSettingIP4ConfigPrivate;

enum {
	PROP_0,
	PROP_DHCP_CLIENT_ID,

	LAST_PROP
};

/**
 * nm_setting_ip4_config_new:
 *
 * Creates a new #NMSettingIP4Config object with default values.
 *
 * Returns: (transfer full): the new empty #NMSettingIP4Config object
 **/
NMSetting *
nm_setting_ip4_config_new (void)
{
	return (NMSetting *) g_object_new (NM_TYPE_SETTING_IP4_CONFIG, NULL);
}

/**
 * nm_setting_ip4_config_get_dhcp_client_id:
 * @setting: the #NMSettingIP4Config
 *
 * Returns the value contained in the #NMSettingIP4Config:dhcp-client-id
 * property.
 *
 * Returns: the configured Client ID to send to the DHCP server when requesting
 * addresses via DHCP.
 **/
const char *
nm_setting_ip4_config_get_dhcp_client_id (NMSettingIP4Config *setting)
{
	g_return_val_if_fail (NM_IS_SETTING_IP4_CONFIG (setting), NULL);

	return NM_SETTING_IP4_CONFIG_GET_PRIVATE (setting)->dhcp_client_id;
}

static gboolean
verify (NMSetting *setting, NMConnection *connection, GError **error)
{
	NMSettingIP4ConfigPrivate *priv = NM_SETTING_IP4_CONFIG_GET_PRIVATE (setting);
	NMSettingIPConfig *s_ip = NM_SETTING_IP_CONFIG (setting);
	NMSettingVerifyResult ret;
	const char *method;

	ret = NM_SETTING_CLASS (nm_setting_ip4_config_parent_class)->verify (setting, connection, error);
	if (ret != NM_SETTING_VERIFY_SUCCESS)
		return ret;

	method = nm_setting_ip_config_get_method (s_ip);
	/* Base class already checked that it exists */
	g_assert (method);

	if (!strcmp (method, NM_SETTING_IP4_CONFIG_METHOD_MANUAL)) {
		if (nm_setting_ip_config_get_num_addresses (s_ip) == 0) {
			g_set_error (error,
			             NM_CONNECTION_ERROR,
			             NM_CONNECTION_ERROR_MISSING_PROPERTY,
			             _("this property cannot be empty for '%s=%s'"),
			             NM_SETTING_IP_CONFIG_METHOD, method);
			g_prefix_error (error, "%s.%s: ", NM_SETTING_IP4_CONFIG_SETTING_NAME, NM_SETTING_IP_CONFIG_ADDRESSES);
			return FALSE;
		}
	} else if (   !strcmp (method, NM_SETTING_IP4_CONFIG_METHOD_LINK_LOCAL)
	           || !strcmp (method, NM_SETTING_IP4_CONFIG_METHOD_SHARED)
	           || !strcmp (method, NM_SETTING_IP4_CONFIG_METHOD_DISABLED)) {
		if (nm_setting_ip_config_get_num_dns (s_ip) > 0) {
			g_set_error (error,
			             NM_CONNECTION_ERROR,
			             NM_CONNECTION_ERROR_INVALID_PROPERTY,
			             _("this property is not allowed for '%s=%s'"),
			             NM_SETTING_IP_CONFIG_METHOD, method);
			g_prefix_error (error, "%s.%s: ", NM_SETTING_IP4_CONFIG_SETTING_NAME, NM_SETTING_IP_CONFIG_DNS);
			return FALSE;
		}

		if (nm_setting_ip_config_get_num_dns_searches (s_ip) > 0) {
			g_set_error (error,
			             NM_CONNECTION_ERROR,
			             NM_CONNECTION_ERROR_INVALID_PROPERTY,
			             _("this property is not allowed for '%s=%s'"),
			             NM_SETTING_IP_CONFIG_METHOD, method);
			g_prefix_error (error, "%s.%s: ", NM_SETTING_IP4_CONFIG_SETTING_NAME, NM_SETTING_IP_CONFIG_DNS_SEARCH);
			return FALSE;
		}

		/* Shared allows IP addresses; link-local and disabled do not */
		if (strcmp (method, NM_SETTING_IP4_CONFIG_METHOD_SHARED) != 0) {
			if (nm_setting_ip_config_get_num_addresses (s_ip) > 0) {
				g_set_error (error,
				             NM_CONNECTION_ERROR,
				             NM_CONNECTION_ERROR_INVALID_PROPERTY,
				             _("this property is not allowed for '%s=%s'"),
				             NM_SETTING_IP_CONFIG_METHOD, method);
				g_prefix_error (error, "%s.%s: ", NM_SETTING_IP4_CONFIG_SETTING_NAME, NM_SETTING_IP_CONFIG_ADDRESSES);
				return FALSE;
			}
		}
	} else if (!strcmp (method, NM_SETTING_IP4_CONFIG_METHOD_AUTO)) {
		/* nothing to do */
	} else {
		g_set_error_literal (error,
		                     NM_CONNECTION_ERROR,
		                     NM_CONNECTION_ERROR_INVALID_PROPERTY,
		                     _("property is invalid"));
		g_prefix_error (error, "%s.%s: ", NM_SETTING_IP4_CONFIG_SETTING_NAME, NM_SETTING_IP_CONFIG_METHOD);
		return FALSE;
	}

	if (priv->dhcp_client_id && !strlen (priv->dhcp_client_id)) {
		g_set_error_literal (error,
		                     NM_CONNECTION_ERROR,
		                     NM_CONNECTION_ERROR_INVALID_PROPERTY,
		                     _("property is empty"));
		g_prefix_error (error, "%s.%s: ", NM_SETTING_IP4_CONFIG_SETTING_NAME, NM_SETTING_IP4_CONFIG_DHCP_CLIENT_ID);
		return FALSE;
	}

	return TRUE;
}

static void
nm_setting_ip4_config_init (NMSettingIP4Config *setting)
{
}

static void
finalize (GObject *object)
{
	NMSettingIP4ConfigPrivate *priv = NM_SETTING_IP4_CONFIG_GET_PRIVATE (object);

	g_free (priv->dhcp_client_id);

	G_OBJECT_CLASS (nm_setting_ip4_config_parent_class)->finalize (object);
}

static void
set_property (GObject *object, guint prop_id,
              const GValue *value, GParamSpec *pspec)
{
	NMSettingIP4ConfigPrivate *priv = NM_SETTING_IP4_CONFIG_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_DHCP_CLIENT_ID:
		g_free (priv->dhcp_client_id);
		priv->dhcp_client_id = g_value_dup_string (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
get_property (GObject *object, guint prop_id,
              GValue *value, GParamSpec *pspec)
{
	NMSettingIP4Config *s_ip4 = NM_SETTING_IP4_CONFIG (object);

	switch (prop_id) {
	case PROP_DHCP_CLIENT_ID:
		g_value_set_string (value, nm_setting_ip4_config_get_dhcp_client_id (s_ip4));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static GVariant *
ip4_dns_to_dbus (const GValue *prop_value)
{
	return nm_utils_ip4_dns_to_variant (g_value_get_boxed (prop_value));
}

static void
ip4_dns_from_dbus (GVariant *dbus_value,
                   GValue *prop_value)
{
	g_value_take_boxed (prop_value, nm_utils_ip4_dns_from_variant (dbus_value));
}

static GVariant *
ip4_addresses_get (NMSetting  *setting,
                   const char *property)
{
	GPtrArray *addrs;
	const char *gateway;
	GVariant *ret;

	g_object_get (setting, property, &addrs, NULL);
	gateway = nm_setting_ip_config_get_gateway (NM_SETTING_IP_CONFIG (setting));
	ret = nm_utils_ip4_addresses_to_variant (addrs, gateway);
	g_ptr_array_unref (addrs);

	return ret;
}

static void
ip4_addresses_set (NMSetting  *setting,
                   GVariant   *connection_dict,
                   const char *property,
                   GVariant   *value)
{
	GPtrArray *addrs;
	GVariant *s_ip4;
	char **labels, *gateway = NULL;
	int i;

	s_ip4 = g_variant_lookup_value (connection_dict, NM_SETTING_IP4_CONFIG_SETTING_NAME, NM_VARIANT_TYPE_SETTING);
	/* If 'address-data' is set then ignore 'addresses' */
	if (g_variant_lookup (s_ip4, "address-data", "aa{sv}", NULL)) {
		g_variant_unref (s_ip4);
		return;
	}

	addrs = nm_utils_ip4_addresses_from_variant (value, &gateway);

	if (g_variant_lookup (s_ip4, "address-labels", "^as", &labels)) {
		for (i = 0; i < addrs->len && labels[i]; i++)
			if (*labels[i])
				nm_ip_address_set_attribute (addrs->pdata[i], "label", g_variant_new_string (labels[i]));
		g_strfreev (labels);
	}

	if (gateway && !g_variant_lookup (s_ip4, "gateway", "s", NULL)) {
		g_object_set (setting,
		              NM_SETTING_IP_CONFIG_GATEWAY, gateway,
		              NULL);
	}
	g_free (gateway);

	g_variant_unref (s_ip4);

	g_object_set (setting, property, addrs, NULL);
	g_ptr_array_unref (addrs);
}

static GVariant *
ip4_address_labels_get (NMSetting    *setting,
                        NMConnection *connection,
                        const char   *property)
{
	NMSettingIPConfig *s_ip = NM_SETTING_IP_CONFIG (setting);
	gboolean have_labels = FALSE;
	GPtrArray *labels;
	GVariant *ret;
	int num_addrs, i;

	num_addrs = nm_setting_ip_config_get_num_addresses (s_ip);
	for (i = 0; i < num_addrs; i++) {
		NMIPAddress *addr = nm_setting_ip_config_get_address (s_ip, i);
		GVariant *label = nm_ip_address_get_attribute (addr, "label");

		if (label) {
			have_labels = TRUE;
			break;
		}
	}
	if (!have_labels)
		return NULL;

	labels = g_ptr_array_sized_new (num_addrs);
	for (i = 0; i < num_addrs; i++) {
		NMIPAddress *addr = nm_setting_ip_config_get_address (s_ip, i);
		GVariant *label = nm_ip_address_get_attribute (addr, "label");

		g_ptr_array_add (labels, (char *) (label ? g_variant_get_string (label, NULL) : ""));
	}

	ret = g_variant_new_strv ((const char * const *) labels->pdata, labels->len);
	g_ptr_array_unref (labels);

	return ret;
}

static GVariant *
ip4_address_data_get (NMSetting    *setting,
                      NMConnection *connection,
                      const char   *property)
{
	GPtrArray *addrs;
	GVariant *ret;

	g_object_get (setting, NM_SETTING_IP_CONFIG_ADDRESSES, &addrs, NULL);
	ret = nm_utils_ip_addresses_to_variant (addrs);
	g_ptr_array_unref (addrs);

	return ret;
}

static void
ip4_address_data_set (NMSetting  *setting,
                      GVariant   *connection_dict,
                      const char *property,
                      GVariant   *value)
{
	GPtrArray *addrs;

	addrs = nm_utils_ip_addresses_from_variant (value, AF_INET);
	g_object_set (setting, NM_SETTING_IP_CONFIG_ADDRESSES, addrs, NULL);
	g_ptr_array_unref (addrs);
}

static GVariant *
ip4_routes_get (NMSetting  *setting,
                const char *property)
{
	GPtrArray *routes;
	GVariant *ret;

	g_object_get (setting, property, &routes, NULL);
	ret = nm_utils_ip4_routes_to_variant (routes);
	g_ptr_array_unref (routes);

	return ret;
}

static void
ip4_routes_set (NMSetting  *setting,
                GVariant   *connection_dict,
                const char *property,
                GVariant   *value)
{
	GPtrArray *routes;
	GVariant *s_ip4;

	s_ip4 = g_variant_lookup_value (connection_dict, NM_SETTING_IP4_CONFIG_SETTING_NAME, NM_VARIANT_TYPE_SETTING);
	/* If 'route-data' is set then ignore 'routes' */
	if (g_variant_lookup (s_ip4, "route-data", "aa{sv}", NULL)) {
		g_variant_unref (s_ip4);
		return;
	}
	g_variant_unref (s_ip4);

	routes = nm_utils_ip4_routes_from_variant (value);
	g_object_set (setting, property, routes, NULL);
	g_ptr_array_unref (routes);
}

static GVariant *
ip4_route_data_get (NMSetting    *setting,
                    NMConnection *connection,
                    const char   *property)
{
	GPtrArray *routes;
	GVariant *ret;

	g_object_get (setting, NM_SETTING_IP_CONFIG_ROUTES, &routes, NULL);
	ret = nm_utils_ip_routes_to_variant (routes);
	g_ptr_array_unref (routes);

	return ret;
}

static void
ip4_route_data_set (NMSetting  *setting,
                    GVariant   *connection_dict,
                    const char *property,
                    GVariant   *value)
{
	GPtrArray *routes;

	routes = nm_utils_ip_routes_from_variant (value, AF_INET);
	g_object_set (setting, NM_SETTING_IP_CONFIG_ROUTES, routes, NULL);
	g_ptr_array_unref (routes);
}


static void
nm_setting_ip4_config_class_init (NMSettingIP4ConfigClass *ip4_class)
{
	NMSettingClass *setting_class = NM_SETTING_CLASS (ip4_class);
	GObjectClass *object_class = G_OBJECT_CLASS (ip4_class);

	g_type_class_add_private (setting_class, sizeof (NMSettingIP4ConfigPrivate));

	/* virtual methods */
	object_class->set_property = set_property;
	object_class->get_property = get_property;
	object_class->finalize     = finalize;
	setting_class->verify = verify;

	/* properties */

	/**
	 * NMSettingIP4Config:dhcp-client-id:
	 *
	 * A string sent to the DHCP server to identify the local machine which the
	 * DHCP server may use to customize the DHCP lease and options.
	 **/
	g_object_class_install_property
		(object_class, PROP_DHCP_CLIENT_ID,
		 g_param_spec_string (NM_SETTING_IP4_CONFIG_DHCP_CLIENT_ID, "", "",
		                      NULL,
		                      G_PARAM_READWRITE |
		                      G_PARAM_STATIC_STRINGS));

	/* IP4-specific property overrides */
	_nm_setting_class_transform_property (setting_class,
	                                      NM_SETTING_IP_CONFIG_DNS,
	                                      G_VARIANT_TYPE ("au"),
	                                      ip4_dns_to_dbus,
	                                      ip4_dns_from_dbus);

	_nm_setting_class_override_property (setting_class,
	                                     NM_SETTING_IP_CONFIG_ADDRESSES,
	                                     G_VARIANT_TYPE ("aau"),
	                                     ip4_addresses_get,
	                                     ip4_addresses_set,
	                                     NULL);

	_nm_setting_class_add_dbus_only_property (setting_class,
	                                          "address-labels",
	                                          G_VARIANT_TYPE_STRING_ARRAY,
	                                          ip4_address_labels_get,
	                                          NULL);

	_nm_setting_class_add_dbus_only_property (setting_class,
	                                          "address-data",
	                                          G_VARIANT_TYPE ("aa{sv}"),
	                                          ip4_address_data_get,
	                                          ip4_address_data_set);

	_nm_setting_class_override_property (setting_class,
	                                     NM_SETTING_IP_CONFIG_ROUTES,
	                                     G_VARIANT_TYPE ("aau"),
	                                     ip4_routes_get,
	                                     ip4_routes_set,
	                                     NULL);

	_nm_setting_class_add_dbus_only_property (setting_class,
	                                          "route-data",
	                                          G_VARIANT_TYPE ("aa{sv}"),
	                                          ip4_route_data_get,
	                                          ip4_route_data_set);

}
