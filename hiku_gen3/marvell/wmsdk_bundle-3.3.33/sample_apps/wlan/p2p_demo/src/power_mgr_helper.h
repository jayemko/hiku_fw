/*
 *  Copyright (C) 2008-2016, Marvell International Ltd.
 *  All Rights Reserved.
 */

#ifndef _POWER_MGR_HELPER_H_
#define _POWER_MGR_HELPER_H_

bool hp_pm_get_wake_status(void);
void hp_pm_reset_wake_status(void);
void hp_pm_set_wake_status(void);
void hp_pm_reset_p2p_timeout_timer();
void hp_pm_stop_p2p_timeout_timer();
bool hp_pm_is_p2p_timeout_occurred();
int hp_pm_init(void);
void hp_pm_wifi_ps_enable();
void hp_pm_wifi_ps_disable();

#endif /* ! _POWER_MGR_HELPER_H_ */
