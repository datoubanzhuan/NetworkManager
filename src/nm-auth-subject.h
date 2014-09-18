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
 * Copyright (C) 2013 Red Hat, Inc.
 */

#ifndef __NETWORKMANAGER_AUTH_SUBJECT_H__
#define __NETWORKMANAGER_AUTH_SUBJECT_H__

#include <config.h>
#include <gio/gio.h>

#if WITH_POLKIT
#include <polkit/polkit.h>
#endif

#include "nm-types.h"

#define NM_TYPE_AUTH_SUBJECT            (nm_auth_subject_get_type ())
#define NM_AUTH_SUBJECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NM_TYPE_AUTH_SUBJECT, NMAuthSubject))
#define NM_AUTH_SUBJECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NM_TYPE_AUTH_SUBJECT, NMAuthSubjectClass))
#define NM_IS_AUTH_SUBJECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NM_TYPE_AUTH_SUBJECT))
#define NM_IS_AUTH_SUBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), NM_TYPE_AUTH_SUBJECT))
#define NM_AUTH_SUBJECT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), NM_TYPE_AUTH_SUBJECT, NMAuthSubjectClass))

struct _NMAuthSubject {
	GObject parent;
};

typedef struct {
	GObjectClass parent;

} NMAuthSubjectClass;

GType nm_auth_subject_get_type (void);

NMAuthSubject *nm_auth_subject_new_from_context (GDBusMethodInvocation *context);

NMAuthSubject *nm_auth_subject_new_from_message (GDBusConnection *connection, GDBusMessage *message);

NMAuthSubject *nm_auth_subject_new_internal (void);

gulong nm_auth_subject_get_uid (NMAuthSubject *subject);

gulong nm_auth_subject_get_pid (NMAuthSubject *subject);

const char *nm_auth_subject_get_dbus_sender (NMAuthSubject *subject);

gboolean nm_auth_subject_get_internal (NMAuthSubject *subject);

#if WITH_POLKIT
PolkitSubject *nm_auth_subject_get_polkit_subject (NMAuthSubject *subject);
#endif

#endif /* __NETWORKMANAGER_AUTH_SUBJECT_H__ */
