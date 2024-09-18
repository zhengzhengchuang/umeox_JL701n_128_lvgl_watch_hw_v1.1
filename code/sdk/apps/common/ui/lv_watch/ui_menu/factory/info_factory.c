#include "info_factory.h"

static const char *DevNameStr = "设备名";
static const char *FwVersionStr = "固件版本";
static const char *AlgoVersionStr = "算法版本";
static const char *TpVersionStr = "TP版本";

static void menu_create_cb(lv_obj_t *obj)
{
    if(!obj) return;

    tileview_register_all_menu(obj, ui_act_id_null, ui_act_id_null, \
        ui_act_id_null, ui_act_id_null, ui_act_id_info_factory);

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

    s16 y = 10;

    /* 设备名 */
    widget_label_para.label_w = 300;
    widget_label_para.label_h = Label_Line_Height;
    widget_label_para.long_mode = LV_LABEL_LONG_SCROLL;
    widget_label_para.text_align = LV_TEXT_ALIGN_CENTER;
    widget_label_para.label_text_color = lv_color_hex(0x777777);
    widget_label_para.label_ver_center = false;
    widget_label_para.user_text_font = NULL;
    widget_label_para.label_parent = obj;
    widget_label_para.label_text = DevNameStr;
    lv_obj_t *dev_name_label = common_widget_label_create(&widget_label_para);
    lv_obj_align(dev_name_label, LV_ALIGN_TOP_MID, 0, y);

    y += Label_Line_Height;
    widget_label_para.label_w = 300;
    widget_label_para.label_h = Label_Line_Height;
    widget_label_para.long_mode = LV_LABEL_LONG_SCROLL;
    widget_label_para.text_align = LV_TEXT_ALIGN_CENTER;
    widget_label_para.label_text_color = lv_color_hex(0xffffff);
    widget_label_para.label_ver_center = false;
    widget_label_para.user_text_font = NULL;
    widget_label_para.label_parent = obj;
    widget_label_para.label_text = GetDevBleName();
    lv_obj_t *dev_name = common_widget_label_create(&widget_label_para);
    lv_obj_align(dev_name, LV_ALIGN_TOP_MID, 0, y);

    /* 固件版本 */
    y += Label_Line_Height;
    widget_label_para.label_w = 300;
    widget_label_para.label_h = Label_Line_Height;
    widget_label_para.long_mode = LV_LABEL_LONG_SCROLL;
    widget_label_para.text_align = LV_TEXT_ALIGN_CENTER;
    widget_label_para.label_text_color = lv_color_hex(0x777777);
    widget_label_para.label_ver_center = false;
    widget_label_para.user_text_font = NULL;
    widget_label_para.label_parent = obj;
    widget_label_para.label_text = FwVersionStr;
    lv_obj_t *fw_version_label = common_widget_label_create(&widget_label_para);
    lv_obj_align(fw_version_label, LV_ALIGN_TOP_MID, 0, y);

    y += Label_Line_Height;
    char *version_str = Fw_Version_Release;
    widget_label_para.label_w = 300;
    widget_label_para.label_h = Label_Line_Height;
    widget_label_para.long_mode = LV_LABEL_LONG_SCROLL;
    widget_label_para.text_align = LV_TEXT_ALIGN_CENTER;
    widget_label_para.label_text_color = lv_color_hex(0xffffff);
    widget_label_para.label_ver_center = false;
    widget_label_para.user_text_font = NULL;
    widget_label_para.label_parent = obj;
    widget_label_para.label_text = Fw_Version_Release;
    lv_obj_t *fw_version = common_widget_label_create(&widget_label_para);
    lv_obj_align(fw_version, LV_ALIGN_TOP_MID, 0, y);

    /* 算法版本 */
    y += Label_Line_Height;
    widget_label_para.label_w = 300;
    widget_label_para.label_h = Label_Line_Height;
    widget_label_para.long_mode = LV_LABEL_LONG_SCROLL;
    widget_label_para.text_align = LV_TEXT_ALIGN_CENTER;
    widget_label_para.label_text_color = lv_color_hex(0x777777);
    widget_label_para.label_ver_center = false;
    widget_label_para.user_text_font = NULL;
    widget_label_para.label_parent = obj;
    widget_label_para.label_text = AlgoVersionStr;
    lv_obj_t *algo_version_label = common_widget_label_create(&widget_label_para);
    lv_obj_align(algo_version_label, LV_ALIGN_TOP_MID, 0, y);

    y += Label_Line_Height;
    int algo[4];
    getReleaseVersion(algo);
    char algo_str[20];
    memset(algo_str, 0, sizeof(algo_str));
    sprintf(algo_str, "V%d.%d.%d_%d", algo[0], algo[1], algo[2], algo[3]);
    widget_label_para.label_w = 300;
    widget_label_para.label_h = Label_Line_Height;
    widget_label_para.long_mode = LV_LABEL_LONG_SCROLL;
    widget_label_para.text_align = LV_TEXT_ALIGN_CENTER;
    widget_label_para.label_text_color = lv_color_hex(0xffffff);
    widget_label_para.label_ver_center = false;
    widget_label_para.user_text_font = NULL;
    widget_label_para.label_parent = obj;
    widget_label_para.label_text = algo_str;
    lv_obj_t *algo_version = common_widget_label_create(&widget_label_para);
    lv_obj_align(algo_version, LV_ALIGN_TOP_MID, 0, y);

    /* TP版本 */
    y += Label_Line_Height;
    widget_label_para.label_w = 300;
    widget_label_para.label_h = Label_Line_Height;
    widget_label_para.long_mode = LV_LABEL_LONG_SCROLL;
    widget_label_para.text_align = LV_TEXT_ALIGN_CENTER;
    widget_label_para.label_text_color = lv_color_hex(0x777777);
    widget_label_para.label_ver_center = false;
    widget_label_para.user_text_font = NULL;
    widget_label_para.label_parent = obj;
    widget_label_para.label_text = TpVersionStr;
    lv_obj_t *tp_version_label = common_widget_label_create(&widget_label_para);
    lv_obj_align(tp_version_label, LV_ALIGN_TOP_MID, 0, y);

    y += Label_Line_Height;
    struct ts_fw_infos tpfw_infos;
    chsc6x_get_chip_info(&tpfw_infos);
    char tp_str[10];
    memset(tp_str, 0, sizeof(tp_str));
    sprintf(tp_str, "0x%x", tpfw_infos.chsc6x_boot_version);
    widget_label_para.label_w = 300;
    widget_label_para.label_h = Label_Line_Height;
    widget_label_para.long_mode = LV_LABEL_LONG_SCROLL;
    widget_label_para.text_align = LV_TEXT_ALIGN_CENTER;
    widget_label_para.label_text_color = lv_color_hex(0xffffff);
    widget_label_para.label_ver_center = false;
    widget_label_para.user_text_font = NULL;
    widget_label_para.label_parent = obj;
    widget_label_para.label_text = tp_str;
    lv_obj_t *tp_version = common_widget_label_create(&widget_label_para);
    lv_obj_align(tp_version, LV_ALIGN_TOP_MID, 0, y);

    
    //printf("yyyyyyyyyyyyyyyyyyyyy = %d\n", y);

    return;
}

static void menu_key_cb(lv_obj_t *obj, int key_value, \
    int key_event)
{
    if(!obj) return;

    return;
}

register_ui_menu_load_info(\
    menu_load_info_factory) = 
{
    .menu_arg = NULL,
    .lock_flag = true,
    .return_flag = false,
    .menu_id = \
        ui_act_id_info_factory,
    .user_offscreen_time = 0,
    .user_refresh_time_inv = 0,
    .key_func_cb = menu_key_cb,
    .create_func_cb = menu_create_cb,
    .destory_func_cb = menu_destory_cb,
    .refresh_func_cb = menu_refresh_cb,
    .display_func_cb = menu_display_cb,
};
