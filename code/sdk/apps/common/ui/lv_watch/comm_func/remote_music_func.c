#include "../lv_watch.h"
#include "../../../../../include_lib/btstack/bluetooth.h"

RmusicInfoPara_t Rmusic_Info;

static const RmusicInfoPara_t InitInfo = {
    .volume = 8,
    .state = Rmusic_puase,
    .title = {0},
};

void DevReqOpRmusicHandle(Rmusic_cmd_t OpCmd, u8 *para)
{
    DevSendRmusicOpCmd(OpCmd, para);
        
    return;
}

void RmusicInfoParaRead(void)
{
    int vm_op_len = sizeof(RmusicInfoPara_t);
    memcpy(&Rmusic_Info, &InitInfo, vm_op_len);
    return;
}


/* ios ams协议 */
static int powi(u8 idx)
{
    int j = 1;
    for(u8 i = 0; i < idx; i++)
        j *= 10;
    return j;
}

void SetAmsMusicTitle(u8 *p_buf, u16 len)
{
    if(len > Rmusic_Title_Len) len = Rmusic_Title_Len;
    memset(Rmusic_Info.title, 0, Rmusic_Title_Len + 1);
    memcpy(Rmusic_Info.title, p_buf, len);
    return;
}

void SetAmsMusicPlayState(char state)
{
    if(state == '1')
        Rmusic_Info.state = Rmusic_play;
    else if(state == '0')
        Rmusic_Info.state = Rmusic_puase;
    return;
}

void SetAmsMusicVolume(char *pstr, u16 len)
{
    static char vol_str[10];
    memset(vol_str, 0, sizeof(vol_str));
    memcpy(vol_str, pstr, len);
    u8 idx = 4;
    int volume = 0;
    for(u16 i = 0; i < len; i++)
    {
        if(vol_str[i] != '.')
        {
            volume += (vol_str[i] - '0')*powi(idx);
            if(idx > 0) idx--;
        }
    }
    volume /= 625;
    volume = volume>Rmusic_Max_Vol?Rmusic_Max_Vol:volume;
    Rmusic_Info.volume = volume;
    return;
}

void DevReqOpAmsHandle(u8 cmd)
{
    ams_send_request_command(cmd);
    return;
}
