#include "../lv_watch.h"
#include "../../../../watch/include/app_task.h"

static u8 ChargePower;
u8 GetChargePowerFlag(void)
{
    return ChargePower;
}

void SetChargePowerFlag(u8 f)
{
    ChargePower = f;
    return;
}

u8 GetChargeState(void)
{
    return get_charge_online_flag();
}

u8 GetChargeFullFlag(void)
{
    u8 full_flag = 0;

    u8 charge_power = GetChargePowerFlag();
    if(charge_power == 1)
    {
        full_flag = get_charge_full_flag();
        goto __end;
    }else
    {   
        int bat_power = GetBatPower();
        if(bat_power == 100)
            full_flag = 1;
        else
            full_flag = 0;
        goto __end;
    }
        
__end:
    return full_flag;
}

void Ldo5vInHandle(void)
{
    DisableGmModule();
    DisablePpgModule(); 
    UserStopwatchReset();
    UserCountdownReset();
    ManualEndSleepHandle();
    
    /* 升级状态不响应充电页面 */
    u8 upg_state = GetOtaUpgradeState();
    if(upg_state != upgrade_none) return;

    /* 表盘传输不响应充电页面 */
    //...

    /* 充电切换到idle任务 */
    // u8 task = app_get_curr_task();
    // if(task != APP_IDLE_TASK)
    //     app_task_switch_to(APP_IDLE_TASK);
    ui_menu_jump(ui_act_id_charge);
 
    return;
}

/* LDOOUT处理 */
void Ldo5vOutHandle(void)
{
    u8 upg_state = GetOtaUpgradeState();
    if(upg_state != upgrade_none) return;

    u8 chg_power = GetChargePowerFlag();
    if(chg_power == 1)
    {
        struct lcd_interface *lcd = lcd_get_hdl();
        if(lcd_sleep_status() == 0) 
            AppCtrlLcdEnterSleep(true);
        cpu_reset();
    }else
    {
        
        /* 拔出切换到蓝牙任务 */
        // u8 task = app_get_curr_task();
        // if(task != APP_BT_TASK)
        //     app_task_switch_to(APP_BT_TASK);

        bool BondFlag = GetDevBondFlag();
        if(BondFlag == false)
        {
            ui_menu_jump(ui_act_id_bond_lang);
        }else
        {
            ui_menu_jump(ui_act_id_watchface);
        }  
    }

    return;
} 