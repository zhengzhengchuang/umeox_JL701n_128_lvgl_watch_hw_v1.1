#include "../lv_watch.h"

#define FifoSize (GoGs_Fifo_WM*6)
static u8 TurnFifoData[FifoSize];
static u8 RaiseFifoData[FifoSize];

static bool WristRaiseAlgo(s16 acc_data)
{
#if 0
    s16 acc_x_max=0,acc_x_min=0,acc_x_up_counter=0;
    static s16 acc_data_x[Raise_Gs_WM*2];
    static s16 data_x0=0,data_x1=0,data_x2=0,data_x3=0,data_x4=0,mean_data=0;
    data_x4=data_x3; data_x3=data_x2; data_x2=data_x1; data_x1=data_x0; data_x0=acc_data;
    mean_data=(data_x0+data_x1+data_x2+data_x3+data_x4)/5;
    for(u8 i=0; i<Raise_Gs_WM*2-1; i++)
        acc_data_x[i]=acc_data_x[i+1];
    acc_data_x[Raise_Gs_WM*2-1]=mean_data;

    for(u8 i=0; i<Raise_Gs_WM*2-1; i++)
    {
        if(acc_data_x[i+1]>acc_data_x[i])
            acc_x_up_counter++;
    }

    //printf("acc_x_up_counter = %d\n", acc_x_up_counter);
    if(acc_x_up_counter >= Raise_Gs_WM*2-2)
    {
        acc_x_max=acc_data_x[0];
        acc_x_min=acc_data_x[0];
        for(u8 i=0; i<Raise_Gs_WM*2; i++)
        {
            if(acc_data_x[i]>acc_x_max)
                acc_x_max=acc_data_x[i];
            if(acc_data_x[i]<acc_x_min)
                acc_x_min=acc_data_x[i];
        }

        s16 diff = acc_x_max-acc_x_min;
        //printf("diff = %d\n", diff);
        if(diff > 4000)
        {
            memset(acc_data_x, 0, sizeof(acc_data_x));
            return true;
        }
    }
    acc_x_up_counter=0;
    return false;
#else
    s16 acc_x_max=0,acc_x_min=0,acc_x_up_counter=0;
    static s16 acc_data_x[14];
    static s16 data_x0=0,data_x1=0,data_x2=0,data_x3=0,data_x4=0,mean_data=0;
    data_x4=data_x3; data_x3=data_x2; data_x2=data_x1; data_x1=data_x0; data_x0=acc_data;
    mean_data=(data_x0+data_x1+data_x2+data_x3+data_x4)/5;
    for(u8 i=0; i<14-1; i++)
        acc_data_x[i]=acc_data_x[i+1];
    acc_data_x[14-1]=mean_data;

    for(u8 i=0; i<14-1; i++)
    {
        if(acc_data_x[i+1]>acc_data_x[i])
            acc_x_up_counter++;
    }

    if(acc_x_up_counter >= 9)
    {
        acc_x_max=acc_data_x[0];
        acc_x_min=acc_data_x[0];
        for(u8 i=0; i<14; i++)
        {
            if(acc_data_x[i]>acc_x_max)
                acc_x_max=acc_data_x[i];
            if(acc_data_x[i]<acc_x_min)
                acc_x_min=acc_data_x[i];
        }

        s16 diff = acc_x_max-acc_x_min;
        //printf("%s:diff = %d\n", __func__, diff);
        if(diff > 4000)
        {
            memset(acc_data_x, 0, sizeof(acc_data_x));
            return true;
        }
    }
    acc_x_up_counter=0;
    return false;
#endif
}

static bool WristTurnAlgo(s16 acc_data)
{
    s16 acc_x_max=0,acc_x_min=0,acc_x_up_counter=0;
    static s16 acc_data_x[14];
    static s16 data_x0=0,data_x1=0,data_x2=0,data_x3=0,data_x4=0,mean_data=0;
    data_x4=data_x3; data_x3=data_x2; data_x2=data_x1; data_x1=data_x0; data_x0=acc_data;
    mean_data=(data_x0+data_x1+data_x2+data_x3+data_x4)/5;
    for(u8 i=0; i<14-1; i++)
        acc_data_x[i]=acc_data_x[i+1];
    acc_data_x[14-1]=mean_data;

    for(u8 i=0; i<14-1; i++)
    {
        if(acc_data_x[i+1]<acc_data_x[i])
            acc_x_up_counter++;
    }

    if(acc_x_up_counter >= 9)
    {
        acc_x_max=acc_data_x[0];
        acc_x_min=acc_data_x[0];
        for(u8 i=0; i<14; i++)
        {
            if(acc_data_x[i]>acc_x_max)
                acc_x_max=acc_data_x[i];
            if(acc_data_x[i]<acc_x_min)
                acc_x_min=acc_data_x[i];
        }

        s16 diff = acc_x_max-acc_x_min;
        //printf("%s:diff = %d\n", __func__, diff);
        if(diff > 4000)
        {
            memset(acc_data_x, 0, sizeof(acc_data_x));
            return true;
        }
    }
    acc_x_up_counter=0;
    return false;
}

extern int lcd_sleep_status();
bool IsWristRaiseBrightScr(u8 *w_buf, u32 w_len)
{
    bool raise = false;

    bool BondFlag = GetDevBondFlag();
    if(BondFlag == false) return raise;

    u8 upg_state = GetOtaUpgradeState();
    if(upg_state != upgrade_none) return raise;

    int raise_sw = GetVmParaCacheByLabel(vm_label_raise);
    if(raise_sw == 0) return raise;

    int dnd_state = GetVmParaCacheByLabel(vm_label_dnd_state);
    if(dnd_state == dnd_state_enable) return raise;

    int lcd_sleep = lcd_sleep_status();
    if(lcd_sleep == 0) return raise;

    if(w_len > FifoSize) w_len = FifoSize;
    memcpy(RaiseFifoData, w_buf, w_len);

    u8 GsData[6];
    u32 r_idx = 0;
    for(u8 i = 0; i < Raise_Gs_WM; i++)
    {
        memcpy(GsData, &RaiseFifoData[r_idx], 6);
        r_idx += 6;
        s16 arry_y = (s16)(((u16)GsData[3]<<8)|(GsData[2]));
        raise = WristRaiseAlgo(-arry_y);
        if(raise == true)
        {
            printf("__________raise\n");
            common_brightscreen_handle();
        }
    }
    
    return raise;
}

bool IsWristTurnOffScr(u8 *w_buf, u32 w_len)
{
    bool turn = false;

    bool BondFlag = GetDevBondFlag();
    if(BondFlag == false) return turn;

    u8 upg_state = GetOtaUpgradeState();
    if(upg_state != upgrade_none) return turn;

    int raise_sw = GetVmParaCacheByLabel(vm_label_raise);
    if(raise_sw == 0) return turn;

    int dnd_state = GetVmParaCacheByLabel(vm_label_dnd_state);
    if(dnd_state == dnd_state_enable) return turn;

    int lcd_sleep = lcd_sleep_status();
    if(lcd_sleep == 1) return turn;

    if(w_len > FifoSize) w_len = FifoSize;
    memcpy(TurnFifoData, w_buf, w_len);

    u8 GsData[6];
    u32 r_idx = 0;
    for(u8 i = 0; i < Raise_Gs_WM; i++)
    {
        memcpy(GsData, &TurnFifoData[r_idx], 6);
        r_idx += 6;
        s16 arry_y = (s16)(((u16)GsData[3]<<8)|(GsData[2]));
        turn = WristTurnAlgo(-arry_y);
        if(turn == true)
        {
            printf("++++++++++++turn\n");
            common_offscreen_handle();
        }
    }

    return turn;
}
