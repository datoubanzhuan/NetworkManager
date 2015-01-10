/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
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
 * Copyright (C) 2014 Red Hat, Inc.
 */

#ifndef NM_SD_ADAPT_H
#define NM_SD_ADAPT_H

#include <config.h>

#include <glib.h>

#include <netinet/in.h>
#include <stdbool.h>
#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <elf.h>
#ifdef HAVE_SYS_AUXV_H
#include <sys/auxv.h>
#endif
#include <unistd.h>
#include <sys/syscall.h>

#include "nm-logging.h"

/*****************************************************************************/

static inline NMLogLevel
_slog_level_to_nm (int slevel)
{
    switch (slevel) {
    case LOG_DEBUG:   return LOGL_DEBUG;
	case LOG_WARNING: return LOGL_WARN;
	case LOG_CRIT:
	case LOG_ERR:     return LOGL_ERR;
	case LOG_INFO:
	case LOG_NOTICE:
	default:          return LOGL_INFO;
	}
}

#define log_internal(level, error, file, line, func, format, ...) \
({ \
	int _nm_e = (error); \
	NMLogLevel _nm_l = _slog_level_to_nm ((level)); \
	if (nm_logging_enabled (_nm_l, LOGD_DHCP)) { \
		const char *_nm_location = strrchr ((""file), '/'); \
		\
		_nm_log (_nm_location ? _nm_location + 1 : (""file), (line), (func), _nm_l, LOGD_DHCP, _nm_e, (""format), ## __VA_ARGS__); \
	} \
	(_nm_e > 0 ? -_nm_e : _nm_e); \
})

#define log_full_errno(level, error, ...) \
({ \
	log_internal(level, error, __FILE__, __LINE__, __func__, __VA_ARGS__); \
})

#define log_assert_failed(text, file, line, func) \
G_STMT_START { \
	log_internal (LOG_CRIT, 0, file, line, func, "Assertion '%s' failed at %s:%u, function %s(). Aborting.", text, file, line, func); \
	g_assert_not_reached (); \
} G_STMT_END

#define log_assert_failed_return(text, file, line, func) \
G_STMT_START { \
	log_internal (LOG_DEBUG, 0, file, line, func, "Assertion '%s' failed at %s:%u, function %s(). Ignoring.", text, file, line, func); \
	g_return_if_fail_warning (G_LOG_DOMAIN, G_STRFUNC, text); \
} G_STMT_END


/*****************************************************************************/

/* Can't include both net/if.h and linux/if.h; so have to define this here */
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#ifndef MAX_HANDLE_SZ
#define MAX_HANDLE_SZ 128
#endif

#define noreturn G_GNUC_NORETURN

#include "sd-id128.h"
#include "sparse-endian.h"
#include "async.h"
#include "util.h"

static inline pid_t gettid(void) {
        return (pid_t) syscall(SYS_gettid);
}

#endif /* NM_SD_ADAPT_H */

