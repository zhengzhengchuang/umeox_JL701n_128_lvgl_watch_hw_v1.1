#include "activity.h"

static void menu_create_cb(lv_obj_t *obj)
{
    if(!obj) return;

    // ui_mode_t ui_mode = p_ui_info_cache->ui_mode;
    // if(ui_mode == ui_mode_watchface)
    // {
    ui_act_id_t left_act_id = ui_act_id_watchface;
    ui_act_id_t right_act_id = ui_act_id_hr_sample;
    tileview_register_all_menu(obj, ui_act_id_null, ui_act_id_null, \
        left_act_id, right_act_id, ui_act_id_activity);
    //}

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

    /* 步数 */
    widget_obj_para.obj_parent = obj;
    widget_obj_para.obj_width = LCD_WIDTH;
    widget_obj_para.obj_height = 100;
    widget_obj_para.obj_bg_opax = LV_OPA_0;
    widget_obj_para.obj_bg_color = lv_color_hex(0x000000);
    widget_obj_para.obj_border_width = 0;
    widget_obj_para.obj_is_scrollable = false;
    widget_obj_para.obj_radius = 0;
    lv_obj_t *step_container = common_widget_obj_create(&widget_obj_para);
    lv_obj_align(step_container, LV_ALIGN_TOP_MID, 0, 32);

    widget_img_para.img_parent = step_container;
    widget_img_para.file_img_dat = activity_00_index;
    widget_img_para.img_click_attr = false;
    widget_img_para.event_cb = NULL;
    lv_obj_t *step_icon = common_widget_img_create(&widget_img_para, NULL);
    lv_obj_align(step_icon, LV_ALIGN_LEFT_MID, 44, 0);

    u32 steps = GetPedoDataSteps();
    widget_data_para.data_x = 108;
    widget_data_para.data_y = 5;
    widget_data_para.num_inv = 2;
    widget_data_para.data_parent = step_container;
    widget_data_para.num_addr_index = comm_num_24x40_wh_00_index;
    widget_data_para.data_align = widget_data_align_left;
    common_data_widget_create(&widget_data_para, widget_data_type_step, &steps);

    widget_label_para.label_w = 200;
    widget_label_para.label_h = Label_Line_Height;
    widget_label_para.long_mode = LV_LABEL_LONG_SCROLL;
    widget_label_para.text_align = LV_TEXT_ALIGN_LEFT;
    widget_label_para.label_text_color = lv_color_hex(0xffffff);
    widget_label_para.label_ver_center = false;
    widget_label_para.user_text_font = NULL;
    widget_label_para.label_parent = step_container;
    widget_label_para.label_text = get_lang_txt_with_id(lang_txtid_steps);
    lv_obj_t *step_label = common_widget_label_create(&widget_label_para);
    lv_obj_align(step_label, LV_ALIGN_BOTTOM_LEFT, 108, -5);

    /* 卡路里 */
    widget_obj_para.obj_parent = obj;
    widget_obj_para.obj_width = LCD_WIDTH;
    widget_obj_para.obj_height = 100;
    widget_obj_para.obj_bg_opax = LV_OPA_0;
    widget_obj_para.obj_bg_color = lv_color_hex(0x000000);
    widget_obj_para.obj_border_width = 0;
    widget_obj_para.obj_is_scrollable = false;
    widget_obj_para.obj_radius = 0;
    lv_obj_t *kcal_container = common_widget_obj_create(&widget_obj_para);
    lv_obj_align(kcal_container, LV_ALIGN_TOP_MID, 0, 156);

    widget_img_para.img_parent = kcal_container;
    widget_img_para.file_img_dat = activity_01_index;
    widget_img_para.img_click_attr = false;
    widget_img_para.event_cb = NULL;
    lv_obj_t *kcal_icon = common_widget_img_create(&widget_img_para, NULL);
    lv_obj_align(kcal_icon, LV_ALIGN_LEFT_MID, 44, 0);

    u32 kcal = GetPedoDataKcal();
    widget_data_para.data_x = 108;
    widget_data_para.data_y = 5;
    widget_data_para.num_inv = 2;
    widget_data_para.data_parent = kcal_container;
    widget_data_para.num_addr_index = comm_num_24x40_wh_00_index;
    widget_data_para.data_align = widget_data_align_left;
    common_data_widget_create(&widget_data_para, widget_data_type_calorie, &kcal);

    widget_label_para.label_w = 200;
    widget_label_para.label_h = Label_Line_Height;
    widget_label_para.long_mode = LV_LABEL_LONG_SCROLL;
    widget_label_para.text_align = LV_TEXT_ALIGN_LEFT;
    widget_label_para.label_text_color = lv_color_hex(0xffffff);
    widget_label_para.label_ver_center = false;
    widget_label_para.user_text_font = NULL;
    widget_label_para.label_parent = kcal_container;
    widget_label_para.label_text = get_lang_txt_with_id(lang_txtid_kcal);
    lv_obj_t *kcal_label = common_widget_label_create(&widget_label_para);
    lv_obj_align(kcal_label, LV_ALIGN_BOTTOM_LEFT, 108, -5);

    /* 距离 */
    int unit_dis = GetVmParaCacheByLabel(vm_label_unit_distance); 

    widget_obj_para.obj_parent = obj;
    widget_obj_para.obj_width = LCD_WIDTH;
    widget_obj_para.obj_height = 100;
    widget_obj_para.obj_bg_opax = LV_OPA_0;
    widget_obj_para.obj_bg_color = lv_color_hex(0x000000);
    widget_obj_para.obj_border_width = 0;
    widget_obj_para.obj_is_scrollable = false;
    widget_obj_para.obj_radius = 0;
    lv_obj_t *distance_container = common_widget_obj_create(&widget_obj_para);
    lv_obj_align(distance_container, LV_ALIGN_TOP_MID, 0, 280);

    widget_img_para.img_parent = distance_container;
    widget_img_para.file_img_dat = activity_02_index;
    widget_img_para.img_click_attr = false;
    widget_img_para.event_cb = NULL;
    lv_obj_t *distance_icon = common_widget_img_create(&widget_img_para, NULL);
    lv_obj_align(distance_icon, LV_ALIGN_LEFT_MID, 44, 0);

    u32 dis_m = GetPedoDataDisM();
    widget_data_para.data_x = 108;
    widget_data_para.data_y = 5;
    widget_data_para.num_inv = 2;
    widget_data_para.data_parent = distance_container;
    widget_data_para.num_addr_index = comm_num_24x40_wh_00_index;
    widget_data_para.data_align = widget_data_align_left;
    common_data_widget_create(&widget_data_para, Widget_Data_type_distance, &dis_m);

    widget_label_para.label_w = 200;
    widget_label_para.label_h = Label_Line_Height;
    widget_label_para.long_mode = LV_LABEL_LONG_SCROLL;
    widget_label_para.text_align = LV_TEXT_ALIGN_LEFT;
    widget_label_para.label_text_color = lv_color_hex(0xffffff);
    widget_label_para.label_ver_center = false;
    widget_label_para.user_text_font = NULL;
    widget_label_para.label_parent = distance_container;
    if(unit_dis == unit_distance_mile)
        widget_label_para.label_text = get_lang_txt_with_id(lang_txtid_mile);
    else
        widget_label_para.label_text = get_lang_txt_with_id(lang_txtid_km);
    lv_obj_t *distance_label = common_widget_label_create(&widget_label_para);
    lv_obj_align(distance_label, LV_ALIGN_BOTTOM_LEFT, 108, -5);

    return;
}

static void menu_key_cb(lv_obj_t *obj, int key_value, \
    int key_event)
{
    if(!obj) return;

    return;
}

register_ui_menu_load_info(\
    menu_load_activity) = 
{
    .menu_arg = NULL,
    .lock_flag = false,
    .return_flag = true,
    .menu_id = \
        ui_act_id_activity,
    .user_offscreen_time = 0,
    .user_refresh_time_inv = 100,
    .key_func_cb = menu_key_cb,
    .create_func_cb = menu_create_cb,
    .destory_func_cb = menu_destory_cb,
    .refresh_func_cb = menu_refresh_cb,
    .display_func_cb = menu_display_cb,
};
