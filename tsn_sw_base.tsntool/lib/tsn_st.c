/*
 * TTTech TSN Library (libtsn)
 * Copyright(c) 2017 TTTech Computertechnik AG.
 *
 * ALL RIGHTS RESERVED.
 * Usage of this software, including source code, netlists, documentation,
 * is subject to restrictions and conditions of the applicable license
 * agreement with TTTech Computertechnik AG or its affiliates.
 *
 * All trademarks used are the property of their respective owners.
 *
 * TTTech Computertechnik AG and its affiliates do not assume any liability
 * arising out of the application or use of any product described or shown
 * herein. TTTech Computertechnik AG and its affiliates reserve the right to
 * make changes, at any time, in order to improve reliability, function or
 * design.
 *
 * Contact Information:
 * support@4me.tttech-dependablenetworks.com
 * TTTech Computertechnik AG, Schoenbrunnerstrasse 7, 1040 Vienna, Austria
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>

#include "tsn_private.h"
#include "tsn_st.h"

#define IEEE8021ST_DIR "ieee8021ST/"

static const uint32_t tsn_module = TSN_ST;

static const sysfs_fn st_fn[MAX_TSN_ST_FUNCTIONS] = {
    [MAX_SDU]                    = IEEE8021ST_DIR"queueMaxSDUTable",
    [OVERRUN]                    = IEEE8021ST_DIR"transmissionOverrunTable",
    [GATE_ENABLED]               = IEEE8021ST_DIR"GateEnabled",
    [ADMIN_GATE_STATES]          = IEEE8021ST_DIR"AdminGateStates",
    [OPER_GATE_STATES]           = IEEE8021ST_DIR"OperGateStates",
    [ADMIN_CONTROL_LIST_LENGTH]  = IEEE8021ST_DIR"AdminControlListLength",
    [OPER_CONTROL_LIST_LENGTH]   = IEEE8021ST_DIR"OperControlListLength",
    [ADMIN_CONTROL_LIST]         = IEEE8021ST_DIR"AdminControlList",
    [OPER_CONTROL_LIST]          = IEEE8021ST_DIR"OperControlList",
    [ADMIN_CYCLE_TIME]           = IEEE8021ST_DIR"AdminCycleTime",
    [OPER_CYCLE_TIME]            = IEEE8021ST_DIR"OperCycleTime",
    [ADMIN_CYCLE_TIME_EXTENSION] = IEEE8021ST_DIR"AdminCycleTimeExtension",
    [OPER_CYCLE_TIME_EXTENSION]  = IEEE8021ST_DIR"OperCycleTimeExtension",
    [ADMIN_BASE_TIME]            = IEEE8021ST_DIR"AdminBaseTime",
    [OPER_BASE_TIME]             = IEEE8021ST_DIR"OperBaseTime",
    [CONFIG_CHANGE]              = IEEE8021ST_DIR"ConfigChange",
    [CONFIG_CHANGE_TIME]         = IEEE8021ST_DIR"ConfigChangeTime",
    [TICK_GRANULARITY]           = IEEE8021ST_DIR"TickGranularity",
    [CURRENT_TIME]               = IEEE8021ST_DIR"CurrentTime",
    [CONFIG_PENDING]             = IEEE8021ST_DIR"ConfigPending",
    [CONFIG_CHANGE_ERROR]        = IEEE8021ST_DIR"ConfigChangeError",
    [SUPPORTED_LIST_MAX]         = IEEE8021ST_DIR"SupportedListMax",
    [ST_SUPPORTED_CYCLE_MAX]     = IEEE8021ST_DIR"SupportedCycleMax",
    [ST_SUPPORTED_INTERVAL_MAX]  = IEEE8021ST_DIR"SupportedIntervalMax"
};

tsn_return_value tsn_st_get_max_sdu(struct port_hdl *port, unsigned int prio,
                                    uint32_t *max_sdu)
{
    tsn_file_desc fd = { .id = MAX_SDU};

    ASSERT_PARAMS(port && max_sdu);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FRDENTRY(fd, max_sdu, prio);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_overrun(struct port_hdl *port, unsigned int prio,
                                    uint64_t *overrun)
{
    tsn_file_desc fd = { .id = OVERRUN};
    ASSERT_PARAMS(port && overrun);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FRDENTRY(fd, overrun, prio);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_set_max_sdu(struct port_hdl *port, unsigned int prio,
                                    const uint32_t max_sdu)
{
    tsn_file_desc fd = { .id = MAX_SDU};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN_W(fd, port, st_fn[fd.id]);
    ASSERT_FWRENTRY(fd, max_sdu, prio);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_gate_enabled(struct port_hdl *port,
                                         bool *gate_enabled)
{
    tsn_file_desc fd = { .id = GATE_ENABLED};
    bool _ge;

    ASSERT_PARAMS(port && gate_enabled);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNu8, 1, (uint8_t *)&_ge);
    *gate_enabled = _ge;
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_set_gate_enabled(struct port_hdl *port,
                                         const bool gate_enabled)
{
    tsn_file_desc fd = { .id = GATE_ENABLED};

    ASSERT_PARAMS(port);
    ASSERT_FOPEN_W(fd, port, st_fn[fd.id]);
    ASSERT_FPRINT(fd, "%"PRIu8, (uint8_t)gate_enabled);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_admin_gate_states(struct port_hdl *port,
                                              uint8_t *admin_gate_state)
{
    tsn_file_desc fd = { .id = ADMIN_GATE_STATES};

    ASSERT_PARAMS(port && admin_gate_state);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNx8, 1, admin_gate_state);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_set_admin_gate_states(struct port_hdl *port,
                                              const uint8_t admin_gate_state)
{
    tsn_file_desc fd = { .id = ADMIN_GATE_STATES};

    ASSERT_PARAMS(port);
    ASSERT_FOPEN_W(fd, port, st_fn[fd.id]);
    ASSERT_FPRINT(fd, "0x%"PRIx8, admin_gate_state);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_oper_gate_states(struct port_hdl *port,
                                             uint8_t *oper_gate_state)
{
    tsn_file_desc fd = { .id = OPER_GATE_STATES};

    ASSERT_PARAMS(port && oper_gate_state);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNx8, 1, oper_gate_state);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_admin_control_list_length(struct port_hdl *port,
                                                      uint32_t *acl_length)
{
    tsn_file_desc fd = { .id = ADMIN_CONTROL_LIST_LENGTH};

    ASSERT_PARAMS(port && acl_length);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, acl_length);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_set_admin_control_list_length(struct port_hdl *port,
                                                      const uint32_t acl_length)
{
    tsn_file_desc fd = { .id = ADMIN_CONTROL_LIST_LENGTH};

    ASSERT_PARAMS(port);
    ASSERT_FOPEN_W(fd, port, st_fn[fd.id]);
    ASSERT_FPRINT(fd, "%"PRIu32, acl_length);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_oper_control_list_length(struct port_hdl *port,
                                                     uint32_t *ocl_length)
{
    tsn_file_desc fd = { .id = OPER_CONTROL_LIST_LENGTH};

    ASSERT_PARAMS(port && ocl_length);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, ocl_length);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_admin_control_list(struct port_hdl *port,
                                               uint32_t entries,
                                               struct st_gate_control_entry *acl_entry)
{
    tsn_file_desc fd = { .id = ADMIN_CONTROL_LIST};

    ASSERT_PARAMS(port && acl_entry);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FRDLIST(fd, acl_entry, entries);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_set_admin_control_list(struct port_hdl *port,
                                               uint32_t entries,
                                               const struct st_gate_control_entry *acl_entry)
{
    tsn_file_desc fd = { .id = ADMIN_CONTROL_LIST};
    uint32_t i;

    ASSERT_PARAMS(port && acl_entry);

    for (i=0; i < entries; i++) {
        if ((acl_entry[i].op != TSN_ST_OP_SET_GATE_STATES) &&
            (acl_entry[i].op != TSN_ST_OP_SET_AND_HOLD_MAC) &&
            (acl_entry[i].op != TSN_ST_OP_SET_AND_RELEASE_MAC))
            return E_TSN_LIB_ST_OP_ERANGE;
    }

    ASSERT_FOPEN_W(fd, port, st_fn[fd.id]);
    ASSERT_FWRLIST(fd, acl_entry, entries);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_oper_control_list(struct port_hdl *port,
                                              uint32_t entries,
                                              struct st_gate_control_entry *ocl_entry)
{
    tsn_file_desc fd = { .id = OPER_CONTROL_LIST};

    ASSERT_PARAMS(port && ocl_entry);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FRDLIST(fd, ocl_entry, entries);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_admin_cycle_time(struct port_hdl *port,
                                             struct tsn_st_rational_number *admin_ctime)
{
    tsn_file_desc fd = { .id = ADMIN_CYCLE_TIME};

    ASSERT_PARAMS(port && admin_ctime);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNu32"/%"SCNu32, 2, &(admin_ctime->numerator), &(admin_ctime->denominator));
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_set_admin_cycle_time(struct port_hdl *port,
                                             const struct tsn_st_rational_number *admin_ctime)
{
    tsn_file_desc fd = { .id = ADMIN_CYCLE_TIME};

    ASSERT_PARAMS(port && admin_ctime);
    ASSERT_FOPEN_W(fd, port, st_fn[fd.id]);
    ASSERT_FPRINT(fd, "%"PRIu32"/%"PRIu32, admin_ctime->numerator, admin_ctime->denominator);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_oper_cycle_time(struct port_hdl *port,
                                            struct tsn_st_rational_number *oper_ctime)
{
    tsn_file_desc fd = { .id = OPER_CYCLE_TIME};

    ASSERT_PARAMS(port && oper_ctime);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNu32"/%"SCNu32, 2, &(oper_ctime->numerator), &(oper_ctime->denominator));
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_admin_cycle_time_extension(struct port_hdl *port,
                                                       uint32_t *admin_ctime_ext)
{
    tsn_file_desc fd = { .id = ADMIN_CYCLE_TIME_EXTENSION};

    ASSERT_PARAMS(port && admin_ctime_ext);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, admin_ctime_ext);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_set_admin_cycle_time_extension(struct port_hdl *port,
                                                       const uint32_t admin_ctime_ext)
{
    tsn_file_desc fd = { .id = ADMIN_CYCLE_TIME_EXTENSION};

    ASSERT_PARAMS(port);
    ASSERT_FOPEN_W(fd, port, st_fn[fd.id]);
    ASSERT_FPRINT(fd, "%"PRIu32, admin_ctime_ext);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_oper_cycle_time_extension(struct port_hdl *port,
                                                      uint32_t *oper_ctime_ext)
{
    tsn_file_desc fd = { .id = OPER_CYCLE_TIME_EXTENSION};

    ASSERT_PARAMS(port && oper_ctime_ext);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, oper_ctime_ext);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_admin_base_time(struct port_hdl *port,
                                            struct tsn_st_ptp_time *admin_btime)
{
    tsn_file_desc fd = { .id = ADMIN_BASE_TIME};

    ASSERT_PARAMS(port && admin_btime);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNu64".%"SCNu32, 2, &(admin_btime->sec), &(admin_btime->nsec));
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_set_admin_base_time(struct port_hdl *port,
                                            const struct tsn_st_ptp_time *admin_btime)
{
    tsn_file_desc fd = { .id = ADMIN_BASE_TIME};

    ASSERT_PARAMS(port && admin_btime);
    ASSERT_FOPEN_W(fd, port, st_fn[fd.id]);
    ASSERT_FPRINT(fd, "%"PRIu64".%09"PRIu32, admin_btime->sec, admin_btime->nsec);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_oper_base_time(struct port_hdl *port,
                                           struct tsn_st_ptp_time *oper_btime)
{
    tsn_file_desc fd = { .id = OPER_BASE_TIME};

    ASSERT_PARAMS(port && oper_btime);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNu64".%"SCNu32, 2, &(oper_btime->sec), &(oper_btime->nsec));
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_config_change(struct port_hdl *port,
                                          bool *config_change)
{
    tsn_file_desc fd = { .id = CONFIG_CHANGE};
    bool _cc;

    ASSERT_PARAMS(port && config_change);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNu8, 1, (uint8_t *)&_cc);
    *config_change = _cc;
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_set_config_change(struct port_hdl *port,
                                          const bool config_change)
{
    tsn_file_desc fd = { .id = CONFIG_CHANGE};

    ASSERT_PARAMS(port);
    ASSERT_FOPEN_W(fd, port, st_fn[fd.id]);
    ASSERT_FPRINT(fd, "%"PRIu8, (uint8_t)config_change);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_config_change_time(struct port_hdl *port,
                                               struct tsn_st_ptp_time *config_change_time)
{
    tsn_file_desc fd = { .id = CONFIG_CHANGE_TIME};

    ASSERT_PARAMS(port && config_change_time);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNu64".%"SCNu32, 2, &(config_change_time->sec),
                 &(config_change_time->nsec));
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_tick_granularity(struct port_hdl *port,
                                             uint32_t *t_granularity)
{
    tsn_file_desc fd = { .id = TICK_GRANULARITY};

    ASSERT_PARAMS(port && t_granularity);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, t_granularity);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_current_time(struct port_hdl *port,
                                         struct tsn_st_ptp_time *current_time)
{
    tsn_file_desc fd = { .id = CURRENT_TIME};

    ASSERT_PARAMS(port && current_time);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNu64".%"SCNu32, 2, &(current_time->sec), &(current_time->nsec));
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_config_pending(struct port_hdl *port,
                                           bool *conf_pending)
{
    tsn_file_desc fd = { .id = CONFIG_PENDING};
    bool _cp;

    ASSERT_PARAMS(port && conf_pending);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNu8, 1, (uint8_t *)&_cp);
    *conf_pending = _cp;
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_config_change_error(struct port_hdl *port,
                                                int *conf_chg_error)
{
    tsn_file_desc fd = { .id = CONFIG_CHANGE_ERROR};

    ASSERT_PARAMS(port && conf_chg_error);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%d", 1, conf_chg_error);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_supported_list_max(struct port_hdl *port,
                                               uint32_t *supp_list_max)
{
    tsn_file_desc fd = { .id = SUPPORTED_LIST_MAX};

    ASSERT_PARAMS(port && supp_list_max);
    ASSERT_FOPEN_R(fd, port, st_fn[fd.id]);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, supp_list_max);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_supported_cycle_max(struct port_hdl *brdev,
                                                struct tsn_st_rational_number *max_cyc)
{
    tsn_file_desc fd = { .id = ST_SUPPORTED_CYCLE_MAX };
    ASSERT_PARAMS(brdev && max_cyc);
    ASSERT_FOPEN(fd, brdev, st_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32 "/%"SCNu32, 2,
                 &max_cyc->numerator, &max_cyc->denominator);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_st_get_supported_interval_max(struct port_hdl *brdev,
                                                   uint32_t *max_int_ns)
{
    tsn_file_desc fd = { .id = ST_SUPPORTED_INTERVAL_MAX };
    ASSERT_PARAMS(brdev && max_int_ns);
    ASSERT_FOPEN(fd, brdev, st_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, max_int_ns);
    ASSERT_FCLOSE(fd);
}

const char *tsn_st_get_known_ernno_msg(tsn_return_value return_code)
{
    (void) return_code;
    return read_st_error_msg(return_code);
}

static uint32_t get_return_header(tsn_file_desc *fd)
{
    return ((fd-> id < MAX_TSN_ST_FUNCTIONS) ?
            (tsn_module<<24)|((fd->id)<<16) : E_TSN_LIB_NO_ERROR);
}
