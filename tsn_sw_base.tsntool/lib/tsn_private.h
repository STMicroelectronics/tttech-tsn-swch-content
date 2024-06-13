/*
 * TTTech TSN Library (libtsn)
 * Copyright(c) 2017 TTTech Industrial Automation AG.
 *
 * ALL RIGHTS RESERVED.
 * Usage of this software, including source code, netlists, documentation,
 * is subject to restrictions and conditions of the applicable license
 * agreement with TTTech Industrial Automation AG or its affiliates.
 *
 * All trademarks used are the property of their respective owners.
 *
 * TTTech Industrial Automation AG and its affiliates do not assume any liability
 * arising out of the application or use of any product described or shown
 * herein. TTTech Industrial Automation AG and its affiliates reserve the right to
 * make changes, at any time, in order to improve reliability, function or
 * design.
 *
 * Contact Information:
 * support@tttech-industrial.com
 * TTTech Industrial Automation AG, Schoenbrunnerstrasse 7, 1040 Vienna, Austria
 */

#ifndef TSN_PRIVATE_H_
#define TSN_PRIVATE_H_

#include <linux/limits.h>
#include <stddef.h>
// cppcheck-suppress  preprocessorErrorDirective
#include <bsd/string.h>
#include "tsn.h"

/** @addtogroup internal */

/** @ingroup internal
    Sysfs directory where port entries are located. */
#define SYSFS_PORT_BASE "/sys/class/net/"

/** @ingroup internal
    Port handler with sysfs port path */
struct port_hdl {
    char sysfs_path[PATH_MAX];
};

/** @ingroup internal
    Port handler with sysfs port path */
typedef char sysfs_fn[FILENAME_MAX];

/** @ingroup internal
    Structure to open the sysfs file using open, id holds function index to report specific known errno codes */
typedef struct tsn_file_descriptor {
    int desc;
    uint32_t id;
    struct port_hdl *port_hdl;
} tsn_file_desc;

typedef char fn_to_use[FILENAME_MAX];

/** @ingroup internal
    Maximum expected values to get from sysfs into an structure */
#define MAX_SYSFS_VALUES        20

/** @ingroup internal
    The following structure is meant to allow to group and read sysfs values toguether and write them in a bulk to a provided structure using ASSERT_RD_STRUCT
    format { sysfspath, MACRO_TO_USE_TO_GET_VALUE,                                               {params_for_macro}, sizeof_field_to_write_to_struct}
                       ^ only supporting ASSERT_SCAN for now, but should be easy to extend       ^union (for now only containing the format parameter)
    The arrays of this structure has to end always with END_OF_STRUCT_FN_ARRAY
*/

struct sysfs_struct_fn {
    sysfs_fn path;
    fn_to_use read_fn;
    union {
        char format[10];
    } args;
    size_t field_size;
};

static uint32_t get_return_header(tsn_file_desc *fd);

/** @ingroup internal
    Must be used to declare the first empty member of a sysfs_struct_fn array
*/
#define END_OF_STRUCT_FN_ARRAY      {"", "", {""}, 0 },

#define RETURN_HEADER               (get_return_header(&fd))

#define ASSERT_FD_RESULT(fd, cond, lib_err)                                    \
        do {                                                                   \
            if (!(cond)) {                                                     \
                uint32_t err = (lib_err);                                      \
                close(fd.desc);                                                \
                return RETURN_HEADER|err;                                      \
            }                                                                  \
        } while (0)

#define ASSERT_FOPEN(fd, port, fn, mode)                                       \
    do {                                                                       \
        char _buf[PATH_MAX+FILENAME_MAX+1];                                    \
        int ret = snprintf(_buf, sizeof(_buf), "%s/%s", port->sysfs_path, fn); \
        if (ret < 0)                                                           \
            return RETURN_HEADER|E_TSN_LIB_NOSUCHFILE|errno;                   \
        if (ret >= (ssize_t)sizeof(_buf))                                      \
            return RETURN_HEADER|E_TSN_LIB_NOSUCHFILE|ENAMETOOLONG;            \
        if ((fd.desc = open(_buf, mode)) < 0)                                  \
            return RETURN_HEADER|E_TSN_LIB_NOSUCHFILE|errno;                   \
    } while (0)

#define ASSERT_FOPEN_R(fd, port, fn) ASSERT_FOPEN(fd, port, fn, O_RDONLY)
#define ASSERT_FOPEN_W(fd, port, fn) ASSERT_FOPEN(fd, port, fn, O_RDWR)

#define ASSERT_FCLOSE(fd)                                                      \
    do {                                                                       \
        if (close(fd.desc))                                                    \
            return RETURN_HEADER|E_TSN_LIB_FCLOSE|errno;                       \
        else                                                                   \
            return E_TSN_LIB_NO_ERROR;                                         \
    } while (0)

#define ASSERT_FSCAN(fd, fmt, items, ...)                                      \
    do {                                                                       \
        char *s = calloc(1, sysconf(_SC_PAGESIZE));                            \
        ASSERT_FD_RESULT(fd, s, E_TSN_LIB_FSCAN|ENOMEM);                       \
        if ((read(fd.desc, s, sysconf(_SC_PAGESIZE)) < 0) ||                   \
            (sscanf(s, fmt, __VA_ARGS__) != (items))) {                        \
            int err = errno;                                                   \
            free(s);                                                           \
            close(fd.desc);                                                    \
            return RETURN_HEADER|E_TSN_LIB_FSCAN|err;                          \
        }                                                                      \
        free(s);                                                               \
    } while (0)

#define ASSERT_FPRINT(fd, fmt, ...)                                            \
    do {                                                                       \
        char *s = calloc(1, sysconf(_SC_PAGESIZE));                            \
        ASSERT_FD_RESULT(fd, s, E_TSN_LIB_FPRINT|ENOMEM);                      \
        int ret = snprintf(s, sysconf(_SC_PAGESIZE), fmt, __VA_ARGS__);        \
        if (ret < 0) {                                                         \
            int err = errno;                                                   \
            free(s);                                                           \
            close(fd.desc);                                                    \
            return RETURN_HEADER|E_TSN_LIB_FPRINT|err;                         \
        }                                                                      \
        if (ret >= sysconf(_SC_PAGESIZE)) {                                    \
            free(s);                                                           \
            close(fd.desc);                                                    \
            return RETURN_HEADER|E_TSN_LIB_FPRINT|EMSGSIZE;                    \
        }                                                                      \
        ssize_t szlen = strlen(s) + 1;                                         \
        ssize_t w_size = write(fd.desc, s, szlen);                             \
        free(s);                                                               \
        ASSERT_FD_RESULT(fd, w_size >= 0, E_TSN_LIB_FPRINT|errno);             \
        ASSERT_FD_RESULT(fd, w_size == szlen, E_TSN_LIB_FPRINT|EMSGSIZE);      \
    } while (0)

#define ASSERT_FRDLIST(fd, buf, n)                                             \
    do {                                                                       \
        /* access to sysfs is limited to one page size */                      \
        uint32_t _nmb = sysconf(_SC_PAGESIZE) / sizeof(*buf);                  \
        ssize_t rd_cnt;                                                        \
        while (n > _nmb) {                                                     \
            rd_cnt = read(fd.desc, buf, _nmb * sizeof(*buf));                  \
            ASSERT_FD_RESULT(fd, rd_cnt >= 0, E_TSN_LIB_FRDLIST|errno);        \
            ASSERT_FD_RESULT(fd, rd_cnt == (ssize_t)(_nmb * sizeof(*buf)),     \
                             E_TSN_LIB_FRDLIST|EMSGSIZE);                      \
            n -= _nmb;                                                         \
            buf += _nmb;                                                       \
        }                                                                      \
        rd_cnt = read(fd.desc, buf, n * sizeof(*buf));                         \
        ASSERT_FD_RESULT(fd, rd_cnt >= 0, E_TSN_LIB_FRDLIST|errno);            \
        ASSERT_FD_RESULT(fd, rd_cnt == (ssize_t)(n * sizeof(*buf)),            \
                         E_TSN_LIB_FRDLIST|EMSGSIZE);                          \
    } while (0)

#define ASSERT_FRDSUBLIST(fd, buf, nsub, skip, n)                              \
    do {                                                                       \
        off_t ofs = lseek(fd.desc, sizeof(*buf) * (nsub) * (skip), SEEK_SET);  \
        ASSERT_FD_RESULT(fd, ofs >= 0, E_TSN_LIB_FRDSUBLIST|errno);            \
        ASSERT_FRDLIST(fd, buf, n);                                            \
    } while (0)

#define ASSERT_FWRLIST(fd, buf, n)                                             \
    do {                                                                       \
        /* access to sysfs is limited to one page size */                      \
        uint32_t _nmb = sysconf(_SC_PAGESIZE) / sizeof(*buf);                  \
        ssize_t wr_cnt;                                                        \
        while (n > _nmb) {                                                     \
            wr_cnt = write(fd.desc, buf, _nmb * sizeof(*buf));                 \
            ASSERT_FD_RESULT(fd, wr_cnt >= 0, E_TSN_LIB_FWRLIST|errno);        \
            ASSERT_FD_RESULT(fd, wr_cnt == (ssize_t)(_nmb * sizeof(*buf)),     \
                             E_TSN_LIB_FWRLIST|EMSGSIZE);                      \
            n -= _nmb;                                                         \
            buf += _nmb;                                                       \
        }                                                                      \
        wr_cnt = write(fd.desc, buf, n * sizeof(*buf));                        \
        ASSERT_FD_RESULT(fd, wr_cnt >= 0, E_TSN_LIB_FWRLIST|errno);            \
        ASSERT_FD_RESULT(fd, wr_cnt == (ssize_t)(n * sizeof(*buf)),            \
                         E_TSN_LIB_FWRLIST|EMSGSIZE);                          \
    } while (0)

#define ASSERT_FWRSUBLIST(fd, buf, nsub, skip, n)                              \
    do {                                                                       \
        off_t ofs = lseek(fd.desc, sizeof(*buf) * (nsub) * (skip), SEEK_SET);  \
        ASSERT_FD_RESULT(fd, ofs >= 0, E_TSN_LIB_FWRSUBLIST|errno);            \
        ASSERT_FWRLIST(fd, buf, n);                                            \
    } while (0)

#define ASSERT_FRDENTRY(fd, pv, idx)                                           \
    do {                                                                       \
        if ((idx) > 0) {                                                       \
            off_t ofs = lseek(fd.desc, (idx) * sizeof(*pv), SEEK_SET);         \
            ASSERT_FD_RESULT(fd, ofs >= 0, E_TSN_LIB_FRDENTRY|errno);          \
        }                                                                      \
        ssize_t rd_cnt = read(fd.desc, pv, sizeof(*pv));                       \
        ASSERT_FD_RESULT(fd, rd_cnt >= 0, E_TSN_LIB_FRDENTRY|errno);           \
        ASSERT_FD_RESULT(fd, rd_cnt == sizeof(*pv),                            \
                         E_TSN_LIB_FRDENTRY|EMSGSIZE);                         \
    } while (0)

#define ASSERT_FWRENTRY(fd, v, idx)                                            \
    do {                                                                       \
        if ((idx) > 0) {                                                       \
            off_t ofs = lseek(fd.desc, (idx) * sizeof(v), SEEK_SET);           \
            ASSERT_FD_RESULT(fd, ofs >= 0, E_TSN_LIB_FWRENTRY|errno);          \
        }                                                                      \
        ssize_t wr_cnt = write(fd.desc, &v, sizeof(v));                        \
        ASSERT_FD_RESULT(fd, wr_cnt >= 0, E_TSN_LIB_FWRENTRY|errno);           \
        ASSERT_FD_RESULT(fd, wr_cnt == sizeof(v), E_TSN_LIB_FWRENTRY|EMSGSIZE);\
    } while (0)

#define ASSERT_RD_STRUCT(fd, port, st, mode, w_st)                             \
    do {                                                                       \
        int read_index = 0;                                                    \
        ptrdiff_t write_index = 0;                                             \
        do {                                                                   \
            ASSERT_FOPEN(fd, port, st[read_index].path, mode);                 \
            if (strcmp(st[read_index].read_fn, "ASSERT_FSCAN") == 0) {         \
                ASSERT_FSCAN(fd, st[read_index].args.format, 1,                \
                            ((ptrdiff_t)w_st+write_index));                    \
            } else {                                                           \
                close(fd.desc);                                                \
                return RETURN_HEADER|E_TSN_LIB_RD_STRUCT;                      \
            }                                                                  \
            write_index += (ptrdiff_t)st[read_index].field_size;               \
            ++read_index;                                                      \
            if ((strcmp(st[read_index].path, "") == 0) ||                      \
                (read_index >= MAX_SYSFS_VALUES)) break;                       \
            close(fd.desc);                                                    \
        } while (1);                                                           \
    } while (0)

#define ASSERT_PARAMS(exp)                                                     \
    if (!(exp))                                                                \
        return E_TSN_LIB_PARAMS;

#endif /*TSN_PRIVATE_H_*/
