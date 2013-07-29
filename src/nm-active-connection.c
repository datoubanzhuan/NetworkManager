/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* NetworkManager -- Network link manager
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Copyright (C) 2008 - 2012 Red Hat, Inc.
 */

#include <glib.h>
#include "nm-types.h"
#include "nm-active-connection.h"
#include "NetworkManager.h"
#include "nm-logging.h"
#include "nm-dbus-glib-types.h"
#include "nm-dbus-manager.h"
#include "nm-device.h"
#include "nm-settings-connection.h"
#include "nm-manager-auth.h"
#include "NetworkManagerUtils.h"

#include "nm-active-connection-glue.h"

/* Base class for anything implementing the Connection.Active D-Bus interface */
G_DEFINE_ABSTRACT_TYPE (NMActiveConnection, nm_active_connection, G_TYPE_OBJECT)

#define NM_ACTIVE_CONNECTION_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                             NM_TYPE_ACTIVE_CONNECTION, \
                                             NMActiveConnectionPrivate))

typedef struct {
	NMConnection *connection;
	char *path;
	char *specific_object;
	NMDevice *device;

	gboolean is_default;
	gboolean is_default6;
	NMActiveConnectionState state;
	gboolean vpn;

	NMAuthSubject *subject;
	NMDevice *master;

	NMAuthChain *chain;
	const char *wifi_shared_permission;
	NMActiveConnectionAuthResultFunc result_func;
	gpointer user_data1;
	gpointer user_data2;
} NMActiveConnectionPrivate;

enum {
	PROP_0,
	PROP_CONNECTION,
	PROP_UUID,
	PROP_SPECIFIC_OBJECT,
	PROP_DEVICES,
	PROP_STATE,
	PROP_DEFAULT,
	PROP_DEFAULT6,
	PROP_VPN,
	PROP_MASTER,

	PROP_INT_CONNECTION,
	PROP_INT_DEVICE,
	PROP_INT_SUBJECT,
	PROP_INT_MASTER,

	LAST_PROP
};

/****************************************************************/

NMActiveConnectionState
nm_active_connection_get_state (NMActiveConnection *self)
{
	return NM_ACTIVE_CONNECTION_GET_PRIVATE (self)->state;
}

void
nm_active_connection_set_state (NMActiveConnection *self,
                                NMActiveConnectionState new_state)
{
	NMActiveConnectionPrivate *priv = NM_ACTIVE_CONNECTION_GET_PRIVATE (self);
	NMActiveConnectionState old_state;

	if (priv->state == new_state)
		return;

	/* DEACTIVATED is a terminal state */
	if (priv->state == NM_ACTIVE_CONNECTION_STATE_DEACTIVATED)
		g_return_if_fail (new_state != NM_ACTIVE_CONNECTION_STATE_DEACTIVATED);

	old_state = priv->state;
	priv->state = new_state;
	g_object_notify (G_OBJECT (self), NM_ACTIVE_CONNECTION_STATE);

	if (   new_state == NM_ACTIVE_CONNECTION_STATE_ACTIVATED
	    || old_state == NM_ACTIVE_CONNECTION_STATE_ACTIVATED) {
		nm_settings_connection_update_timestamp (NM_SETTINGS_CONNECTION (priv->connection),
		                                         (guint64) time (NULL), TRUE);
	}

	if (priv->state == NM_ACTIVE_CONNECTION_STATE_DEACTIVATED) {
		/* Device is no longer relevant when deactivated */
		g_clear_object (&priv->device);
		g_object_notify (G_OBJECT (self), NM_ACTIVE_CONNECTION_DEVICES);
	}
}

const char *
nm_active_connection_get_name (NMActiveConnection *self)
{
	g_return_val_if_fail (NM_IS_ACTIVE_CONNECTION (self), NULL);

	return nm_connection_get_id (NM_ACTIVE_CONNECTION_GET_PRIVATE (self)->connection);
}

NMConnection *
nm_active_connection_get_connection (NMActiveConnection *self)
{
	return NM_ACTIVE_CONNECTION_GET_PRIVATE (self)->connection;
}

const char *
nm_active_connection_get_path (NMActiveConnection *self)
{
	return NM_ACTIVE_CONNECTION_GET_PRIVATE (self)->path;
}

const char *
nm_active_connection_get_specific_object (NMActiveConnection *self)
{
	return NM_ACTIVE_CONNECTION_GET_PRIVATE (self)->specific_object;
}

void
nm_active_connection_set_specific_object (NMActiveConnection *self,
                                          const char *specific_object)
{
	NMActiveConnectionPrivate *priv = NM_ACTIVE_CONNECTION_GET_PRIVATE (self);

	/* Nothing that calls this function should be using paths from D-Bus,
	 * where NM uses "/" to mean NULL.
	 */
	g_assert (g_strcmp0 (specific_object, "/") != 0);

	if (g_strcmp0 (priv->specific_object, specific_object) == 0)
		return;

	g_free (priv->specific_object);
	priv->specific_object = g_strdup (specific_object);
	g_object_notify (G_OBJECT (self), NM_ACTIVE_CONNECTION_SPECIFIC_OBJECT);
}

void
nm_active_connection_set_default (NMActiveConnection *self, gboolean is_default)
{
	NMActiveConnectionPrivate *priv;

	g_return_if_fail (NM_IS_ACTIVE_CONNECTION (self));

	priv = NM_ACTIVE_CONNECTION_GET_PRIVATE (self);
	if (priv->is_default == is_default)
		return;

	priv->is_default = is_default;
	g_object_notify (G_OBJECT (self), NM_ACTIVE_CONNECTION_DEFAULT);
}

gboolean
nm_active_connection_get_default (NMActiveConnection *self)
{
	g_return_val_if_fail (NM_IS_ACTIVE_CONNECTION (self), FALSE);

	return NM_ACTIVE_CONNECTION_GET_PRIVATE (self)->is_default;
}

void
nm_active_connection_set_default6 (NMActiveConnection *self, gboolean is_default6)
{
	NMActiveConnectionPrivate *priv;

	g_return_if_fail (NM_IS_ACTIVE_CONNECTION (self));

	priv = NM_ACTIVE_CONNECTION_GET_PRIVATE (self);
	if (priv->is_default6 == is_default6)
		return;

	priv->is_default6 = is_default6;
	g_object_notify (G_OBJECT (self), NM_ACTIVE_CONNECTION_DEFAULT6);
}

gboolean
nm_active_connection_get_default6 (NMActiveConnection *self)
{
	g_return_val_if_fail (NM_IS_ACTIVE_CONNECTION (self), FALSE);

	return NM_ACTIVE_CONNECTION_GET_PRIVATE (self)->is_default6;
}

void
nm_active_connection_export (NMActiveConnection *self)
{
	NMActiveConnectionPrivate *priv = NM_ACTIVE_CONNECTION_GET_PRIVATE (self);
	static guint32 counter = 0;

	g_assert (priv->device || priv->vpn);

	priv->path = g_strdup_printf (NM_DBUS_PATH "/ActiveConnection/%d", counter++);
	nm_dbus_manager_register_object (nm_dbus_manager_get (), priv->path, self);
}

NMAuthSubject *
nm_active_connection_get_subject (NMActiveConnection *self)
{
	g_return_val_if_fail (NM_IS_ACTIVE_CONNECTION (self), NULL);

	return NM_ACTIVE_CONNECTION_GET_PRIVATE (self)->subject;
}

gboolean
nm_active_connection_get_user_requested (NMActiveConnection *self)
{
	g_return_val_if_fail (NM_IS_ACTIVE_CONNECTION (self), FALSE);

	return !nm_auth_subject_get_internal (NM_ACTIVE_CONNECTION_GET_PRIVATE (self)->subject);
}

gulong
nm_active_connection_get_user_uid (NMActiveConnection *self)
{
	NMActiveConnectionPrivate *priv;

	g_return_val_if_fail (NM_IS_ACTIVE_CONNECTION (self), G_MAXULONG);
	priv = NM_ACTIVE_CONNECTION_GET_PRIVATE (self);

	return nm_auth_subject_get_uid (priv->subject);
}

NMDevice *
nm_active_connection_get_device (NMActiveConnection *self)
{
	g_return_val_if_fail (NM_IS_ACTIVE_CONNECTION (self), NULL);

	return NM_ACTIVE_CONNECTION_GET_PRIVATE (self)->device;
}

NMDevice *
nm_active_connection_get_master (NMActiveConnection *self)
{
	g_return_val_if_fail (NM_IS_ACTIVE_CONNECTION (self), NULL);

	return NM_ACTIVE_CONNECTION_GET_PRIVATE (self)->master;
}

/**
 * nm_active_connection_set_master:
 * @self: the #NMActiveConnection
 * @master: if the activation depends on another device (ie, bond or bridge
 *    master to which this device will be enslaved) pass the #NMDevice that this
 *    activation request be enslaved to
 *
 * Sets the master device of the active connection.
 */
void
nm_active_connection_set_master (NMActiveConnection *self, NMDevice *master)
{
	NMActiveConnectionPrivate *priv;

	g_return_if_fail (NM_IS_ACTIVE_CONNECTION (self));
	g_return_if_fail (NM_IS_DEVICE (self));

	priv = NM_ACTIVE_CONNECTION_GET_PRIVATE (self);
	/* Master is write-once, and must be set before exporting the object */
	g_return_if_fail (priv->master == NULL);
	g_return_if_fail (priv->path == NULL);
	g_return_if_fail (master != priv->device);

	priv->master = g_object_ref (master);
}

/****************************************************************/

static void
auth_done (NMAuthChain *chain,
           GError *error,
           DBusGMethodInvocation *unused,
           gpointer user_data)
{
	NMActiveConnection *self = NM_ACTIVE_CONNECTION (user_data);
	NMActiveConnectionPrivate *priv = NM_ACTIVE_CONNECTION_GET_PRIVATE (self);
	NMAuthCallResult result;

	g_assert (priv->chain == chain);
	g_assert (priv->result_func != NULL);

	/* Must stay alive over the callback */
	g_object_ref (self);

	if (error) {
		priv->result_func (self, FALSE, error->message, priv->user_data1, priv->user_data2);
		goto done;
	}

	/* Caller has had a chance to obtain authorization, so we only need to
	 * check for 'yes' here.
	 */
	result = nm_auth_chain_get_result (chain, NM_AUTH_PERMISSION_NETWORK_CONTROL);
	if (result != NM_AUTH_CALL_RESULT_YES) {
		priv->result_func (self,
		                   FALSE,
		                   "Not authorized to control networking.",
		                   priv->user_data1,
		                   priv->user_data2);
		goto done;
	}

	if (priv->wifi_shared_permission) {
		result = nm_auth_chain_get_result (chain, priv->wifi_shared_permission);
		if (result != NM_AUTH_CALL_RESULT_YES) {
			priv->result_func (self,
			                   FALSE,
			                   "Not authorized to share connections via wifi.",
			                   priv->user_data1,
			                   priv->user_data2);
			goto done;
		}
	}

	/* Otherwise authorized and available to activate */
	priv->result_func (self, TRUE, NULL, priv->user_data1, priv->user_data2);

done:
	nm_auth_chain_unref (chain);
	priv->chain = NULL;
	priv->result_func = NULL;
	priv->user_data1 = NULL;
	priv->user_data2 = NULL;

	g_object_unref (self);
}

/**
 * nm_active_connection_authorize:
 * @self: the #NMActiveConnection
 * @result_func: function to be called on success or error
 * @user_data1: pointer passed to @result_func
 * @user_data2: additional pointer passed to @result_func
 *
 * Checks whether the subject that initiated the active connection (read from
 * the #NMActiveConnection::subject property) is authorized to complete this
 * activation request.
 */
void
nm_active_connection_authorize (NMActiveConnection *self,
                                NMActiveConnectionAuthResultFunc result_func,
                                gpointer user_data1,
                                gpointer user_data2)
{
	NMActiveConnectionPrivate *priv = NM_ACTIVE_CONNECTION_GET_PRIVATE (self);
	const char *wifi_permission = NULL;

	g_return_if_fail (result_func != NULL);
	g_return_if_fail (priv->chain == NULL);

	priv->chain = nm_auth_chain_new_subject (priv->subject, NULL, auth_done, self);
	g_assert (priv->chain);

	/* Check that the subject is allowed to use networking at all */
	nm_auth_chain_add_call (priv->chain, NM_AUTH_PERMISSION_NETWORK_CONTROL, TRUE);

	/* Shared wifi connections require special permissions too */
	wifi_permission = nm_utils_get_shared_wifi_permission (priv->connection);
	if (wifi_permission) {
		priv->wifi_shared_permission = wifi_permission;
		nm_auth_chain_add_call (priv->chain, wifi_permission, TRUE);
	}

	/* Wait for authorization */
	priv->result_func = result_func;
	priv->user_data1 = user_data1;
	priv->user_data2 = user_data2;
}

/****************************************************************/

static void
nm_active_connection_init (NMActiveConnection *self)
{
}

static void
constructed (GObject *object)
{
	G_OBJECT_CLASS (nm_active_connection_parent_class)->constructed (object);
	g_assert (NM_ACTIVE_CONNECTION_GET_PRIVATE (object)->subject);
}

static void
set_property (GObject *object, guint prop_id,
			  const GValue *value, GParamSpec *pspec)
{
	NMActiveConnectionPrivate *priv = NM_ACTIVE_CONNECTION_GET_PRIVATE (object);
	const char *tmp;

	switch (prop_id) {
	case PROP_INT_CONNECTION:
		g_warn_if_fail (priv->connection == NULL);
		priv->connection = g_value_dup_object (value);
		break;
	case PROP_INT_DEVICE:
		g_warn_if_fail (priv->device == NULL);
		priv->device = g_value_dup_object (value);
		if (priv->device)
			g_warn_if_fail (priv->device != priv->master);
		break;
	case PROP_INT_SUBJECT:
		priv->subject = g_value_dup_object (value);
		break;
	case PROP_INT_MASTER:
		nm_active_connection_set_master (NM_ACTIVE_CONNECTION (object), g_value_get_object (value));
		break;
	case PROP_SPECIFIC_OBJECT:
		tmp = g_value_get_boxed (value);
		/* NM uses "/" to mean NULL */
		if (g_strcmp0 (tmp, "/") != 0)
			priv->specific_object = g_value_dup_boxed (value);
		break;
	case PROP_DEFAULT:
		priv->is_default = g_value_get_boolean (value);
		break;
	case PROP_DEFAULT6:
		priv->is_default6 = g_value_get_boolean (value);
		break;
	case PROP_VPN:
		priv->vpn = g_value_get_boolean (value);
		break;
	case PROP_MASTER:
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
	NMActiveConnectionPrivate *priv = NM_ACTIVE_CONNECTION_GET_PRIVATE (object);
	GPtrArray *devices;

	switch (prop_id) {
	case PROP_CONNECTION:
		g_value_set_boxed (value, nm_connection_get_path (priv->connection));
		break;
	case PROP_UUID:
		g_value_set_string (value, nm_connection_get_uuid (priv->connection));
		break;
	case PROP_SPECIFIC_OBJECT:
		g_value_set_boxed (value, priv->specific_object ? priv->specific_object : "/");
		break;
	case PROP_DEVICES:
		devices = g_ptr_array_sized_new (1);
		if (priv->device)
			g_ptr_array_add (devices, g_strdup (nm_device_get_path (priv->device)));
		g_value_take_boxed (value, devices);
		break;
	case PROP_STATE:
		g_value_set_uint (value, priv->state);
		break;
	case PROP_DEFAULT:
		g_value_set_boolean (value, priv->is_default);
		break;
	case PROP_DEFAULT6:
		g_value_set_boolean (value, priv->is_default6);
		break;
	case PROP_VPN:
		g_value_set_boolean (value, priv->vpn);
		break;
	case PROP_MASTER:
		g_value_set_boxed (value, priv->master ? nm_device_get_path (priv->master) : "/");
		break;
	case PROP_INT_SUBJECT:
		g_value_set_object (value, priv->subject);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
dispose (GObject *object)
{
	NMActiveConnectionPrivate *priv = NM_ACTIVE_CONNECTION_GET_PRIVATE (object);

	if (priv->chain) {
		nm_auth_chain_unref (priv->chain);
		priv->chain = NULL;
	}

	g_free (priv->path);
	priv->path = NULL;
	g_free (priv->specific_object);
	priv->specific_object = NULL;

	g_clear_object (&priv->connection);
	g_clear_object (&priv->device);
	g_clear_object (&priv->master);
	g_clear_object (&priv->subject);

	G_OBJECT_CLASS (nm_active_connection_parent_class)->dispose (object);
}

static void
nm_active_connection_class_init (NMActiveConnectionClass *ac_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (ac_class);

	g_type_class_add_private (ac_class, sizeof (NMActiveConnectionPrivate));

	/* virtual methods */
	object_class->get_property = get_property;
	object_class->set_property = set_property;
	object_class->constructed = constructed;
	object_class->dispose = dispose;

	/* D-Bus exported properties */
	g_object_class_install_property (object_class, PROP_CONNECTION,
		g_param_spec_boxed (NM_ACTIVE_CONNECTION_CONNECTION,
		                    "Connection",
		                    "Connection",
		                    DBUS_TYPE_G_OBJECT_PATH,
		                    G_PARAM_READABLE));

	g_object_class_install_property (object_class, PROP_UUID,
		g_param_spec_string (NM_ACTIVE_CONNECTION_UUID,
		                     "Connection UUID",
		                     "Connection UUID",
		                     NULL,
		                     G_PARAM_READABLE));

	g_object_class_install_property (object_class, PROP_SPECIFIC_OBJECT,
		g_param_spec_boxed (NM_ACTIVE_CONNECTION_SPECIFIC_OBJECT,
		                    "Specific object",
		                    "Specific object",
		                    DBUS_TYPE_G_OBJECT_PATH,
		                    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class, PROP_DEVICES,
		g_param_spec_boxed (NM_ACTIVE_CONNECTION_DEVICES,
		                    "Devices",
		                    "Devices",
		                    DBUS_TYPE_G_ARRAY_OF_OBJECT_PATH,
		                    G_PARAM_READABLE));

	g_object_class_install_property (object_class, PROP_STATE,
		g_param_spec_uint (NM_ACTIVE_CONNECTION_STATE,
		                   "State",
		                   "State",
		                   NM_ACTIVE_CONNECTION_STATE_UNKNOWN,
		                   NM_ACTIVE_CONNECTION_STATE_DEACTIVATING,
		                   NM_ACTIVE_CONNECTION_STATE_UNKNOWN,
		                   G_PARAM_READABLE));

	g_object_class_install_property (object_class, PROP_DEFAULT,
		g_param_spec_boolean (NM_ACTIVE_CONNECTION_DEFAULT,
		                      "Default",
		                      "Is the default IPv4 active connection",
		                      FALSE,
		                      G_PARAM_READWRITE));

	g_object_class_install_property (object_class, PROP_DEFAULT6,
		g_param_spec_boolean (NM_ACTIVE_CONNECTION_DEFAULT6,
		                      "Default6",
		                      "Is the default IPv6 active connection",
		                      FALSE,
		                      G_PARAM_READWRITE));

	g_object_class_install_property (object_class, PROP_VPN,
		g_param_spec_boolean (NM_ACTIVE_CONNECTION_VPN,
		                      "VPN",
		                      "Is a VPN connection",
		                      FALSE,
		                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class, PROP_MASTER,
		g_param_spec_boxed (NM_ACTIVE_CONNECTION_MASTER,
		                    "Master",
		                    "Path of master device",
		                    DBUS_TYPE_G_OBJECT_PATH,
		                    G_PARAM_READABLE));

	/* Internal properties */
	g_object_class_install_property (object_class, PROP_INT_CONNECTION,
		g_param_spec_object (NM_ACTIVE_CONNECTION_INT_CONNECTION,
		                     "Internal Connection",
		                     "Internal connection",
		                     NM_TYPE_CONNECTION,
		                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class, PROP_INT_DEVICE,
		g_param_spec_object (NM_ACTIVE_CONNECTION_INT_DEVICE,
		                     "Internal device",
		                     "Internal device",
		                     NM_TYPE_DEVICE,
		                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class, PROP_INT_SUBJECT,
		g_param_spec_object (NM_ACTIVE_CONNECTION_INT_SUBJECT,
		                     "Subject",
		                     "Subject",
		                     NM_TYPE_AUTH_SUBJECT,
		                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class, PROP_INT_MASTER,
		g_param_spec_object (NM_ACTIVE_CONNECTION_INT_MASTER,
		                     "Internal master device",
		                     "Internal device",
		                     NM_TYPE_DEVICE,
		                     G_PARAM_READWRITE));

	nm_dbus_manager_register_exported_type (nm_dbus_manager_get (),
	                                        G_TYPE_FROM_CLASS (ac_class),
	                                        &dbus_glib_nm_active_connection_object_info);
}

