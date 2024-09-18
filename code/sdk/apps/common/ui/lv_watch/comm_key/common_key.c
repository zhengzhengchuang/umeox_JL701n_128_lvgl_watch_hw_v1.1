#include "app_task.h"
#include "../lv_watch.h"
#include "../../../../watch/include/app_main.h"
#include "../../../../../include_lib/system/event.h"
#include "../../../../../include_lib/btstack/le/le_user.h"
#include "../../../../watch/include/ui/lcd_spi/lcd_drive.h"

static bool menu_timer_lock_flag = false;

bool get_menu_timer_lock_flag(void)
{
    return menu_timer_lock_flag;
}

void set_menu_timer_lock_flag(bool flag)
{
    menu_timer_lock_flag = flag;

    return;
}

void common_key_msg_handle(int key_value, int key_event)
{
    bool power_on = GetPowerOnStatus();
    if(power_on == true) return;
    
    printf("key_value = %d, key_event = %d\n", key_value, key_event);

    u8 chg_state = GetChargeState();
    bool BondFlag = GetDevBondFlag();
    ui_act_id_t cur_act_id = p_ui_info_cache->cur_act_id;
    bool menu_lock = p_ui_info_cache->menu_load_info.lock_flag;

    if(lcd_sleep_status())
    {
        if(key_value >= Common_Key_Val_Max)
            return;

        if(key_event == KEY_EVENT_CLICK)
        {
            /************按键亮屏显示页************/
            common_brightscreen_handle();
        }
    }else
    {
        /*******亮屏按键操作时，需重置刷新定时器*******/
        //common_refresh_timer_re_run();

        /*******亮屏按键操作时，需重置熄屏定时器*******/
        common_offscreen_timer_restart();

        if(key_value == Common_Key_Val_1)//电源键
        {
            if(key_event == KEY_EVENT_CLICK)
            {
                common_offscreen_handle();
            }else if(key_event == KEY_EVENT_LONG_3S)
            {
                if(BondFlag == false)
                {
                    if(chg_state == 0)
                        ui_menu_jump(ui_act_id_device_op);
                }else
                {
                    DevOpMenuPopUp();
                }    
            }
        }else if(key_value == Common_Key_Val_0)//Home键
        {
            if(key_event == KEY_EVENT_CLICK)
            {
                if(cur_act_id == ui_act_id_watchface)
                {
                    /*在表盘页面下，菜单键点击进入菜单列表*/
                    ui_menu_jump(ui_act_id_menu);
                    return;
                }else
                {
                    // if(cur_act_id == ui_act_id_bond_lang && BondFlag == false)
                    // {
                    //     //处于未绑定开机语言选择页面
                    //     return;
                    // }

                    if(menu_lock == false)
                    {
                        ui_menu_jump(ui_act_id_watchface);
                        return;
                    }
                }
            }else if(key_event == KEY_EVENT_DOUBLE_CLICK)
            {
#if TCFG_APP_PC_EN
                if (pc_app_check() == false) {
                    printf("pc dev check error!!!!!!!\n");
                } else {
                    app_task_switch_to(APP_PC_TASK);
                }
#else
                if(cur_act_id == ui_act_id_menu)
                {
                    /*在菜单页面，双击更换风格*/
                    int cur_menu = \
                        GetVmParaCacheByLabel(vm_label_menu_view);
                    cur_menu++;
                    cur_menu %= ui_menu_view_num;
                    SetVmParaCacheByLabel(vm_label_menu_view, cur_menu);
                    ui_menu_jump(ui_act_id_menu);
                    return;
                }
#if 0               
                else if(cur_act_id == ui_act_id_about)
                {
                    //测试用
                    ui_menu_jump(ui_act_id_factory);
                }
#endif
#endif
            }
#if 1
            else if(key_event == KEY_EVENT_FIRTH_CLICK)
            {
                //bool BondFlag = GetDevBondFlag();
                if(BondFlag == false && cur_act_id == ui_act_id_dev_bond)
                    ui_menu_jump(ui_act_id_watchface);
            }
#endif
        }else
        {
            return;
        }

        /*******按键无通用功能，回调到页面做相应地处理*******/
        lv_obj_t *obj = tileview_get_center_menu();
        ui_menu_key_func_cb key_func_cb = \
            p_ui_info_cache->menu_load_info.key_func_cb;
        if(key_func_cb)
            key_func_cb(obj, key_value, key_event);
    }

    return;
}
