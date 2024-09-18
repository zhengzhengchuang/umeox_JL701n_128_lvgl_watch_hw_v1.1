#include "gm_test.h"

static void menu_create_cb(lv_obj_t *obj)
{
    if(!obj) return;

    ui_act_id_t prev_act_id = ui_act_id_about;
    tileview_register_all_menu(obj, ui_act_id_null, ui_act_id_null, \
        prev_act_id, ui_act_id_null, ui_act_id_gm_test);

    return;
}

static void menu_destory_cb(lv_obj_t *obj)
{
    return;
}

static void menu_refresh_cb(lv_obj_t *obj)
{
    if(!obj) return;

    return;
}

static void menu_display_cb(lv_obj_t *obj)
{
    if(!obj) return;

#if GM_DATA_DEBUG
    for(u16 i = 0; i < GM_LEN; i++)
    {
        printf("%d:[0] = %f, [1] = %f, [2] = %f\n", i, gm_data0[i], gm_data1[i], gm_data2[i]);
    }
#endif

    return;
}

static void menu_key_cb(lv_obj_t *obj, int key_value, \
    int key_event)
{
    if(!obj) return;

    return;
}

register_ui_menu_load_info(\
    menu_load_gm_test) = 
{
    .menu_arg = NULL,
    .lock_flag = false,
    .return_flag = true,
    .menu_id = \
        ui_act_id_gm_test,
    .user_offscreen_time = 0,
    .user_refresh_time_inv = 0,
    .key_func_cb = menu_key_cb,
    .create_func_cb = menu_create_cb,
    .destory_func_cb = menu_destory_cb,
    .refresh_func_cb = menu_refresh_cb,
    .display_func_cb = menu_display_cb,
};
