/* SPDX-License-Identifier: GPL-2.0 */
/*
 * TTTech EDGE/DE-IP Linux driver
 * Copyright(c) 2018 TTTech Industrial Automation AG.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 * Contact Information:
 * support@tttech-industrial.com
 * TTTech Industrial Automation AG, Schoenbrunnerstrasse 7, 1040 Vienna, Austria
 */

#ifndef _EDGE_SCHED_H
#define _EDGE_SCHED_H

struct edgx_sched_rational {
	u32 num;
	u32 denom;
} __packed;

struct edgx_sched_tab_entry {
	u32 time_interval;
	u8 operation_name;
	u8 gate_states;
	u16 padding;
} __packed;

struct edgx_sched_limits {
	u32 hw_granularity_ns;
	u32 freq_err_abs;
	u32 max_entry_cnt;
	u32 max_int_len;
	s64 max_cyc_ms;
	u8 nr_gates;
};

struct edgx_sched_com;
struct edgx_sched;
struct edgx_ifdesc;

struct edgx_sched_event {
	struct edgx_sched *sched;
};

struct edgx_sched_ops {
	/* Get schedule offset relative to BaseTime */
	int (*get_sched_offset)(struct edgx_sched_event *sched_evt,
				s64 *offset_ns);
	/* Prepare final configuration to write to HW */
	int (*prepare_config)(struct edgx_sched_event *sched_evt,
			      const struct edgx_sched_tab_entry *entries,
			      size_t count);
	/* Notify after switch to new configuration */
	void (*config_changed)(struct edgx_sched_event *sched_evt);
};

enum edgx_sched_sel { EDGX_SCHED_ADMIN, EDGX_SCHED_OPER };

extern const struct edgx_sched_tab_entry edgx_sched_undef_entry;

int edgx_sched_com_probe(struct edgx_br *br, struct edgx_br_irq *irq,
			 enum edgx_br_irq_nr irq_nr,
			 const char *drv_name,
			 const struct edgx_ifdesc *ifd_com,
			 struct edgx_sched_com **psc,
			 const struct edgx_sched_ops *ops,
			 u16 init_gate_states);
void edgx_sched_com_set_params(struct edgx_sched_com *sched_com,
			       u32 freq_err_abs, u32 sched_granul);
void edgx_sched_com_shutdown(struct edgx_sched_com *sched_com);

int edgx_probe_sched(struct edgx_sched_com *sched_com,
		     unsigned int sched_idx,
		     struct edgx_sched_event *sched_evt);
void edgx_shutdown_sched(struct edgx_sched *sched);

unsigned int edgx_sched_get_idx(struct edgx_sched *sched);

void edgx_sched_lock(struct edgx_sched *sched);
void edgx_sched_unlock(struct edgx_sched *sched);

struct edgx_br *edgx_sched_get_br(struct edgx_sched *sched);
void edgx_sched_set_delay(struct edgx_sched *sched, u8 delay);
void edgx_sched_set_g_close_adv(struct edgx_sched *sched,
				u16 g_cl_adv_lo, u16 g_cl_adv_hi);
int edgx_sched_get_s2g(struct edgx_sched *sched, u64 clk,
		       u64 *s2g_min, u64 *s2g_max);
int edgx_sched_get_free_tab(struct edgx_sched *sched);
int edgx_sched_get_used_tab(struct edgx_sched *sched);

int edgx_sched_write_entry(struct edgx_sched *sched,
			   unsigned int tab_idx, unsigned int row_idx,
			   u16 gate_states, u32 time_ns);
int edgx_sched_read_entry(struct edgx_sched *sched,
			  unsigned int tab_idx, unsigned int row_idx,
			  u16 *gate_states, u32 *time_ns);

const struct edgx_sched_limits *edgx_sched_get_limits(struct edgx_sched *sched);
int edgx_sched_get_current_time(struct edgx_sched *sched,
				struct timespec64 *ts);
void edgx_sched_rational_to_nsec(const struct edgx_sched_rational *rat,
				 u64 *nsec,
				 u32 *subnsec);
void edgx_sched_nsec_to_rational(u64 nsec, u32 subnsec,
				 struct edgx_sched_rational *rational);

int edgx_sched_set_gate_enabled(struct edgx_sched *sched, bool enabled);
bool edgx_sched_get_gate_enabled_locked(struct edgx_sched *sched);
bool edgx_sched_get_gate_enabled(struct edgx_sched *sched);
int edgx_sched_set_admin_gate_states(struct edgx_sched *sched, u8 gate_states,
				     u8 mask);
u8 edgx_sched_get_admin_gate_states(struct edgx_sched *sched, u8 mask);
int edgx_sched_set_cycle_time(struct edgx_sched *sched,
			      const struct edgx_sched_rational *ct);
int edgx_sched_get_cycle_time_locked(struct edgx_sched *sched,
				     enum edgx_sched_sel sel,
				     struct edgx_sched_rational *ct);
int edgx_sched_get_cycle_time(struct edgx_sched *sched,
			      enum edgx_sched_sel sel,
			      struct edgx_sched_rational *ct);
int edgx_sched_set_base_time(struct edgx_sched *sched,
			     const struct timespec64 *base_time);
int edgx_sched_get_base_time(struct edgx_sched *sched,
			     enum edgx_sched_sel sel,
			     struct timespec64 *base_time);
int edgx_sched_set_ctrl_list_len(struct edgx_sched *sched,
				 u32 list_len);
int edgx_sched_get_ctrl_list_len(struct edgx_sched *sched,
				 enum edgx_sched_sel sel,
				 u32 *list_len);
int edgx_sched_set_admin_ctrl_list(struct edgx_sched *sched,
				   size_t first,
				   const struct edgx_sched_tab_entry *entries,
				   size_t count);
int edgx_sched_get_admin_ctrl_list(struct edgx_sched *sched,
				   size_t first,
				   struct edgx_sched_tab_entry *entries,
				   size_t count);
int edgx_sched_set_config_change(struct edgx_sched *sched, bool config_change);
int edgx_sched_get_config_change(struct edgx_sched *sched, bool *config_change);
int edgx_sched_get_config_change_time(struct edgx_sched *sched,
				      struct timespec64 *cct);
int edgx_sched_get_config_pending(struct edgx_sched *sched, bool *pending);
int edgx_sched_get_config_change_error(struct edgx_sched *sched,
				       u64 *cc_error);

#endif /* _EDGE_SCHED_H */
