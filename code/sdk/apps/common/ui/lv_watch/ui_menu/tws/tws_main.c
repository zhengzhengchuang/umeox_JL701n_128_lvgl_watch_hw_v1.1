#include "tws_main.h"

static int16_t scroll_y;
static int16_t scroll_top_y;
static int16_t scroll_bottom_y;
static lv_obj_t *list_ctx_container;
static lv_obj_t *elem_container[Elem_Num];

//static uint8_t DevNum;
static uint8_t ConnState;
//static bool bt_en_state = true;

static const uint8_t ec_idx[Elem_Num] = {
    0, 1, 2, 3, 4, 
};

static void add_cb(lv_event_t *e)
{
    if(!e) return;

    TwsBtEmitterStart();

    ui_menu_jump(ui_act_id_tws_list);

    return;
}

static void tws_add_create(lv_obj_t *obj, uint8_t menu_align)
{
    widget_img_para.img_parent = obj;
    widget_img_para.file_img_dat = comm_icon_05_index;
    widget_img_para.img_click_attr = false;
    widget_img_para.event_cb = NULL;
    lv_obj_t *add_container = common_widget_img_create(&widget_img_para, NULL);
    lv_obj_align(add_container, LV_ALIGN_TOP_MID, 0, 52);

    widget_label_para.label_parent = add_container;
    widget_label_para.label_w = 220;
    widget_label_para.label_h = Label_Line_Height;
    widget_label_para.long_mode = LV_LABEL_LONG_SCROLL;
    if(menu_align == menu_align_right)
        widget_label_para.text_align = LV_TEXT_ALIGN_RIGHT;
    else
        widget_label_para.text_align = LV_TEXT_ALIGN_LEFT;
    widget_label_para.label_text_color = lv_color_hex(0xffffff);
    widget_label_para.label_ver_center = false;
    widget_label_para.user_text_font = NULL;
    widget_label_para.label_text = get_lang_txt_with_id(lang_txtid_add_new_devices);
    lv_obj_t *add_label = common_widget_label_create(&widget_label_para);
    if(menu_align == menu_align_right)
        lv_obj_align(add_label, LV_ALIGN_RIGHT_MID, -20, 0);
    else
        lv_obj_align(add_label, LV_ALIGN_LEFT_MID, 20, 0);

    widget_img_para.img_parent = add_container;
    widget_img_para.file_img_dat = tws_00_index;
    widget_img_para.img_click_attr = true;
    widget_img_para.event_cb = add_cb;
    widget_img_para.user_data = NULL;  
    lv_obj_t *add_icon = common_widget_img_create(&widget_img_para, NULL);
    if(menu_align == menu_align_right)
        lv_obj_align(add_icon, LV_ALIGN_LEFT_MID, 20, 0);
    else
        lv_obj_align(add_icon, LV_ALIGN_RIGHT_MID, -20, 0);
    lv_obj_set_ext_click_area(add_icon, 20);

    return;
}

static void list_ctx_container_cb(lv_event_t *e)
{
    if(!e) return;

    lv_obj_t *obj = lv_event_get_target(e);
    scroll_y = lv_obj_get_scroll_y(obj);

    return;
}

static void list_ctx_container_create(lv_obj_t *obj)
{
    widget_obj_para.obj_parent = obj;
    widget_obj_para.obj_x = 0;
    widget_obj_para.obj_y = 140;
    widget_obj_para.obj_width = LCD_WIDTH;
    widget_obj_para.obj_height = LCD_HEIGHT - 140;
    widget_obj_para.obj_bg_opax = LV_OPA_0;
    widget_obj_para.obj_bg_color = lv_color_hex(0x000000);
    widget_obj_para.obj_border_opax = LV_OPA_0;
    widget_obj_para.obj_border_width = 0;
    widget_obj_para.obj_border_color = lv_color_hex(0x000000);
    widget_obj_para.obj_radius = 0;
    widget_obj_para.obj_is_scrollable = true;
    list_ctx_container = common_widget_obj_create(&widget_obj_para);
    lv_obj_set_style_pad_bottom(list_ctx_container, 25, LV_PART_MAIN);
    lv_obj_add_event_cb(list_ctx_container, list_ctx_container_cb, LV_EVENT_SCROLL, NULL);

    return;
}

static void elem_container_cb(lv_event_t *e)
{
    if(!e) return;

    uint8_t idx = *(uint8_t *)lv_event_get_user_data(e);
    if(idx == TwsConn.dev)
    {
        ui_menu_jump(ui_act_id_tws_disc);
        goto __end;
    }

    u8 *mac = TwsPara.Paired[idx].mac;
    TwsBtEmitterConnByMac(mac);

__end:
    return;
}

/* 长按回调 */
static void elem_container_long_cb(lv_event_t *e)
{
    if(!e) return;

    uint8_t idx = *(uint8_t *)lv_event_get_user_data(e);
    uint8_t num = TwsPara.num;
    if(idx >= num) return;

    set_unpair_dev(idx);
    ui_menu_jump(ui_act_id_tws_unpair);

__end:
    return;
}

static void elem_container_create(void)
{
    uint8_t num = TwsPara.num;

    widget_obj_para.obj_parent = list_ctx_container;
    widget_obj_para.obj_width = LCD_WIDTH;
    widget_obj_para.obj_height = 80;
    widget_obj_para.obj_bg_opax = LV_OPA_0;
    widget_obj_para.obj_bg_color = lv_color_hex(0x000000);
    widget_obj_para.obj_border_opax = LV_OPA_0;
    widget_obj_para.obj_border_width = 0;
    widget_obj_para.obj_border_color = lv_color_hex(0x000000);
    widget_obj_para.obj_radius = 0;
    widget_obj_para.obj_is_scrollable = false;

    for(uint8_t idx = 0; idx < num; idx++)
    {
        widget_obj_para.obj_x = 0;
        widget_obj_para.obj_y = 64 + idx*80;
        elem_container[idx] = common_widget_obj_create(&widget_obj_para);
        lv_obj_add_event_cb(elem_container[idx], elem_container_cb, LV_EVENT_SHORT_CLICKED, (void *)&ec_idx[idx]);
        lv_obj_add_event_cb(elem_container[idx], elem_container_long_cb, LV_EVENT_LONG_PRESSED, (void *)&ec_idx[idx]);
    }

    return;
}

static void elem_ctx_create(uint8_t menu_align)
{
    uint8_t num = TwsPara.num;

    if(num == 0) return;

    widget_label_para.label_parent = list_ctx_container;
    widget_label_para.label_w = 300;
    widget_label_para.label_h = Label_Line_Height;
    widget_label_para.long_mode = LV_LABEL_LONG_SCROLL;
    widget_label_para.text_align = LV_TEXT_ALIGN_CENTER;
    widget_label_para.label_text_color = lv_color_hex(0xffffff);
    widget_label_para.label_ver_center = true;
    widget_label_para.user_text_font = NULL;
    widget_label_para.label_text = get_lang_txt_with_id(lang_txtid_paired_tws);
    lv_obj_t *paired_label = common_widget_label_create(&widget_label_para);
    lv_obj_align(paired_label, LV_ALIGN_TOP_MID, 0, 10);

    elem_container_create();

    for(int8_t i = num - 1; i >= 0; i--)
    {
        widget_img_para.img_parent = elem_container[i];
        widget_img_para.file_img_dat = tws_01_index;
        widget_img_para.img_click_attr = false;
        widget_img_para.event_cb = NULL;
        lv_obj_t *tws_icon = common_widget_img_create(&widget_img_para, NULL);
        if(menu_align == menu_align_right)
            lv_obj_align(tws_icon, LV_ALIGN_RIGHT_MID, -20, 0);
        else
            lv_obj_align(tws_icon, LV_ALIGN_LEFT_MID, 20, 0);

        widget_label_para.label_parent = elem_container[i];
        widget_label_para.label_w = 260;
        widget_label_para.label_h = Label_Line_Height;
        widget_label_para.long_mode = LV_LABEL_LONG_SCROLL;
        if(menu_align == menu_align_right)
            widget_label_para.text_align = LV_TEXT_ALIGN_RIGHT;
        else
            widget_label_para.text_align = LV_TEXT_ALIGN_LEFT;
        if(i == TwsConn.dev)
            widget_label_para.label_text_color = lv_color_hex(0xffffff);
        else
            widget_label_para.label_text_color = lv_color_hex(0x666666);
        widget_label_para.label_ver_center = false;
        widget_label_para.user_text_font = NULL;
        widget_label_para.label_text = TwsPara.Paired[i].name;
        lv_obj_t *name_label = common_widget_label_create(&widget_label_para);
        if(menu_align == menu_align_right)
            lv_obj_align(name_label, LV_ALIGN_RIGHT_MID, -86, 0);
        else
            lv_obj_align(name_label, LV_ALIGN_LEFT_MID, 86, 0);
    }

    return;
}

static void menu_create_cb(lv_obj_t *obj)
{
    if(!obj) return;

    ui_act_id_t prev_act_id = ui_act_id_tool_box;
    if(!lang_txt_is_arabic())
        tileview_register_all_menu(obj, ui_act_id_null, ui_act_id_null, \
            prev_act_id, ui_act_id_null, ui_act_id_tws_main);
    else
        tileview_register_all_menu(obj, ui_act_id_null, ui_act_id_null, \
            ui_act_id_null, prev_act_id, ui_act_id_tws_main);

    return;
}

static void menu_destory_cb(lv_obj_t *obj)
{
    return;
}

static void menu_refresh_cb(lv_obj_t *obj)
{
    if(!obj) return;

    ui_act_id_t act_id = p_ui_info_cache->cur_act_id;

    // bool bt_en = GetBtEnableState();
    // if(bt_en == false)
    // {
    //     TwsBtUserCloseHandle();
    //     ui_menu_jump(act_id);
    //     goto __end;
    // }

    bool NewDev = GetNewPairedDevFlag();
    if(NewDev == true)
    {
        ui_menu_jump(act_id);
        goto __end;
    }

    if(ConnState != TwsConn.state)
    {
        ui_menu_jump(act_id);
        goto __end;
    }

__end:
    return;
}

static void menu_display_cb(lv_obj_t *obj)
{
    if(!obj) return;

    //bt_en_state = GetBtEnableState();
    ConnState = TwsConn.state;
    SetNewPairedDevFlag(false);

    menu_align_t menu_align = menu_align_left;
    if(lang_txt_is_arabic())
        menu_align = menu_align_right;

    tws_add_create(obj, menu_align);

    list_ctx_container_create(obj);

    elem_ctx_create(menu_align);

    lv_obj_scroll_to_y(list_ctx_container, 0, LV_ANIM_OFF);
    lv_obj_update_layout(list_ctx_container);
    scroll_top_y = lv_obj_get_scroll_top(list_ctx_container);
    scroll_bottom_y = lv_obj_get_scroll_bottom(list_ctx_container);
    scroll_y = scroll_y<scroll_top_y?scroll_top_y:scroll_y;
    scroll_y = scroll_y>scroll_bottom_y?scroll_bottom_y:scroll_y;
    lv_obj_scroll_to_y(list_ctx_container, scroll_y, LV_ANIM_OFF);

    return;
}

static void menu_key_cb(lv_obj_t *obj, int key_value, \
    int key_event)
{
    if(!obj) return;

    return;
}

static void menu_rdec_cb(lv_obj_t *obj, int state)
{
    if(!obj) return;

    if(lv_anim_get(list_ctx_container, NULL))
        return;

    int16_t scroll_y_tmp = scroll_y;

    if(state == Rdec_Forward)
        scroll_y_tmp += 100;
    else if(state == Rdec_Backward)
        scroll_y_tmp -= 100;
    else
        return;

    scroll_y_tmp = scroll_y_tmp < scroll_top_y?scroll_top_y:scroll_y_tmp;
    scroll_y_tmp = scroll_y_tmp > scroll_bottom_y?scroll_bottom_y:scroll_y_tmp;
    if(scroll_y_tmp != scroll_y)
        lv_obj_scroll_to_y(list_ctx_container, scroll_y_tmp, LV_ANIM_ON);

    return;
}

register_ui_menu_load_info(\
    menu_load_tws_main) = 
{
    .menu_arg = NULL,
    .lock_flag = false,
    .return_flag = true,
    .menu_id = \
        ui_act_id_tws_main,
    .user_offscreen_time = 0,
    .user_refresh_time_inv = 200,
    .key_func_cb = menu_key_cb,
    .rdec_func_cb = menu_rdec_cb,
    .create_func_cb = menu_create_cb,
    .destory_func_cb = menu_destory_cb,
    .refresh_func_cb = menu_refresh_cb,
    .display_func_cb = menu_display_cb,
};
