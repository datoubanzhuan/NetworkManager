/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
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
 * Copyright 2004 - 2014 Red Hat, Inc.
 */

#ifndef __NM_ERRORS_H__
#define __NM_ERRORS_H__

/**
 * NMConnectionError:
 * @NM_CONNECTION_ERROR_FAILED: unknown or unclassified error
 * @NM_CONNECTION_ERROR_SETTING_NOT_FOUND: the #NMConnection object
 *   did not contain the specified #NMSetting object
 * @NM_CONNECTION_ERROR_PROPERTY_NOT_FOUND: the #NMConnection did not contain the
 *   requested #NMSetting property
 * @NM_CONNECTION_ERROR_PROPERTY_NOT_SECRET: an operation which requires a secret
 *   was attempted on a non-secret property
 * @NM_CONNECTION_ERROR_MISSING_SETTING: the #NMConnection object is missing an
 *   #NMSetting which is required for its configuration. The error message will
 *   always be prefixed with "<setting-name>: ", where "<setting-name>" is the
 *   name of the setting that is missing.
 * @NM_CONNECTION_ERROR_INVALID_SETTING: the #NMConnection object contains an
 *   invalid or inappropriate #NMSetting. The error message will always be
 *   prefixed with "<setting-name>: ", where "<setting-name>" is the name of the
 *   setting that is invalid.
 * @NM_CONNECTION_ERROR_MISSING_PROPERTY: the #NMConnection object is invalid
 *   because it is missing a required property. The error message will always be
 *   prefixed with "<setting-name>.<property-name>: ", where "<setting-name>" is
 *   the name of the setting with the missing property, and "<property-name>" is
 *   the property that is missing.
 * @NM_CONNECTION_ERROR_INVALID_PROPERTY: the #NMConnection object is invalid
 *   because a property has an invalid value. The error message will always be
 *   prefixed with "<setting-name>.<property-name>: ", where "<setting-name>" is
 *   the name of the setting with the invalid property, and "<property-name>" is
 *   the property that is invalid.
 *
 * Describes errors that may result from operations involving a #NMConnection
 * or its #NMSettings.
 *
 * These errors may be returned directly from #NMConnection and #NMSetting
 * methods, or may be returned from D-Bus operations (eg on #NMClient or
 * #NMDevice), where they correspond to errors in the
 * "org.freedesktop.NetworkManager.Settings.Connection" namespace.
 */
typedef enum {
	NM_CONNECTION_ERROR_FAILED = 0,                   /*< nick=Failed >*/
	NM_CONNECTION_ERROR_SETTING_NOT_FOUND,            /*< nick=SettingNotFound >*/
	NM_CONNECTION_ERROR_PROPERTY_NOT_FOUND,           /*< nick=PropertyNotFound >*/
	NM_CONNECTION_ERROR_PROPERTY_NOT_SECRET,          /*< nick=PropertyNotSecret >*/
	NM_CONNECTION_ERROR_MISSING_SETTING,              /*< nick=MissingSetting >*/
	NM_CONNECTION_ERROR_INVALID_SETTING,              /*< nick=InvalidSetting >*/
	NM_CONNECTION_ERROR_MISSING_PROPERTY,             /*< nick=MissingProperty >*/
	NM_CONNECTION_ERROR_INVALID_PROPERTY,             /*< nick=InvalidProperty >*/
} NMConnectionError;

#define NM_CONNECTION_ERROR nm_connection_error_quark ()
GQuark nm_connection_error_quark (void);

/**
 * NMCryptoError:
 * @NM_CRYPTO_ERROR_FAILED: generic failure
 * @NM_CRYPTO_ERROR_INVALID_DATA: the certificate or key data provided
 *   was invalid
 * @NM_CRYPTO_ERROR_INVALID_PASSWORD: the password was invalid
 * @NM_CRYPTO_ERROR_UNKNOWN_CIPHER: the data uses an unknown cipher
 * @NM_CRYPTO_ERROR_DECRYPTION_FAILED: decryption failed
 * @NM_CRYPTO_ERROR_ENCRYPTION_FAILED: encryption failed
 *
 * Cryptography-related errors that can be returned from some nm-utils methods,
 * and some #NMSetting8021x operations.
 */
typedef enum {
	NM_CRYPTO_ERROR_FAILED = 0,
	NM_CRYPTO_ERROR_INVALID_DATA,
	NM_CRYPTO_ERROR_INVALID_PASSWORD,
	NM_CRYPTO_ERROR_UNKNOWN_CIPHER,
	NM_CRYPTO_ERROR_DECRYPTION_FAILED,
	NM_CRYPTO_ERROR_ENCRYPTION_FAILED,
} NMCryptoError;

#define NM_CRYPTO_ERROR nm_crypto_error_quark ()
GQuark nm_crypto_error_quark (void);

#endif /* __NM_ERRORS_H__ */
