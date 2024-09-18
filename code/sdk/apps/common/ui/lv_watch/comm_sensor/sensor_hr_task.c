
#include "../lv_watch.h"

extern void hx3605_module_start(uint8_t work_type);
extern void hx3605_module_stop(void);
extern void hx3605_agc_timeout_handle(void);
extern void hx3605_ppg_timeout_handle(void);

/****gomore与加速度计参数配套*****/
#define FifoSize (HrGs_Fifo_WM*6*5)
static u32 Wlen;
static u32 Rlen;
static u32 Widx;
static u32 Ridx;
static u8 WFifoData[FifoSize];

#define RMaxCnt (12)
#define RFifoLen (RMaxCnt*6)
static u8 RFifoData[RFifoLen];

//static u8 FifoWFlag;

static void HrGsFifoParaInit(void)
{
    Wlen = 0; Rlen = 0;
    Widx = 0; Ridx = 0;
    memset(WFifoData, 0, sizeof(WFifoData));
    memset(RFifoData, 0, sizeof(RFifoData));

    return;
}

static void PpgEnableModuleHandle(int *msg)
{
    HrGsFifoParaInit();
    SetPpgUnwearCnt(0);
    SetPpgWearStatus(true);
    SetPpgEnableFlag(true);

    u8 work_type = msg[1];
    SetPpgWorkType(work_type);
    hx3605_module_start(work_type);
 
    return;
}

static void PpgDisableModuleHandle(void)
{
    hx3605_module_stop();
    SetPpgWorkType(PpgWorkNone);
    SetPpgEnableFlag(false);

    return;
}

static void PpgTask(void *p)
{
    int ret;
    int msg[32] = {0};

    HrGsFifoParaInit();
    DisablePpgModule();

    while(1)
    {
        ret = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg)); 

        if(ret == OS_TASKQ)
            PpgTaskMsgHandle(msg, ARRAY_SIZE(msg));
    }
    
    return;
}

void PpgTaskCreate(void)
{
    int err = task_create(PpgTask, NULL, Ppg_Task_Name);
    if(err) printf("ppg task create err!!!!!!!:%d \n", err);

    return;
}

void PpgTaskMsgHandle(int *msg, u8 len)
{
    if(!msg || len == 0)
        return;

    int cmd = msg[0];

    switch(cmd)
    {
        case PpgMsgEnable:
            PpgEnableModuleHandle(msg);  
            break;

        case PpgMsgDisable:
            PpgDisableModuleHandle();
            break;

        case PpgMsgAgcTimeout:
            hx3605_agc_timeout_handle();
            break;

        case PpgMsgPpgTimeout:
            hx3605_ppg_timeout_handle();
            break;

        default:
            break;
    }

    return;
}

int PostPpgTaskMsg(int *msg, u8 len)
{
    int err = 0;
    int count = 0;

    if(!msg || len == 0)
        return -1;

__retry:
    err = os_taskq_post_type(Ppg_Task_Name, \
        msg[0], len - 1, &msg[1]);

    if(cpu_in_irq() || cpu_irq_disabled())
        return err;

    if(err) 
    {
        if(!strcmp(os_current_task(), Ppg_Task_Name)) 
            return err;

        if(count > 20)
            return -1;
        
        count++;
        os_time_dly(1);
        goto __retry;
    }

    return err;
}

void DisablePpgModule(void)
{
    int PpgMsg[2];
	PpgMsg[0] = PpgMsgDisable;
	PostPpgTaskMsg(PpgMsg, 1); 

    return;
}

void EnablePpgModule(u8 type, u8 mode)
{
    int PpgMsg[2];
	PpgMsg[0] = PpgMsgEnable;
    PpgMsg[1] = (int)type;
	PostPpgTaskMsg(PpgMsg, 2);

    SetPpgMode(mode);

    return;
}

static u8 PpgMode = PpgModeManual;
u8 GetPpgMode(void)
{
    return PpgMode;
}
void SetPpgMode(u8 mode)
{
    PpgMode = mode;
    return;
}

static u8 PpgWork = PpgWorkNone;
u8 GetPpgWorkType(void)
{
    return PpgWork;
}
void SetPpgWorkType(u8 work)
{
    PpgWork = work;
    return;
}

static bool PpgEn = false; 
bool GetPpgEnableFlag(void)
{
    return PpgEn;
}
void SetPpgEnableFlag(bool en)
{
    PpgEn = en;
    return;
}

static u8 UnwearCnt;
u8 GetPpgUnwearCnt(void)
{
    return UnwearCnt;
}

void SetPpgUnwearCnt(u8 cnt)
{
    UnwearCnt = cnt;
    return;
}

static bool PpgWear = true;
bool GetPpgWearStatus(void)
{
    return PpgWear;
}
void SetPpgWearStatus(bool wear)
{
    PpgWear = wear;
    return;
}

void HrGsDataFifoWrite(u8 *w_buf, u32 w_len)
{
    bool BondFlag = GetDevBondFlag();
    if(BondFlag == false) return;

    bool PpgEn = GetPpgEnableFlag();
    if(PpgEn == false) return;

    //FifoWFlag = 1;

    u32 idx = 0;
    u8 *pW = WFifoData;
    if(Widx + w_len <= FifoSize)
    {
        memcpy(&pW[Widx], &w_buf[idx], w_len);
        Widx += w_len;
    }else
    {
        u32 remain = FifoSize - Widx;
        memcpy(&pW[Widx], &w_buf[idx], remain);
        idx += remain;

        Widx = 0;
        remain = w_len - remain;//20
        memcpy(&pW[Widx], &w_buf[idx], remain);
        Widx += remain;
    }
    Wlen += w_len;

    //FifoWFlag = 0;

    return;
}

#define HrGsR (10)
u32 HrGsDataFifoRead(s16 *acc_x, s16 *acc_y, s16 *acc_z)
{
    u32 count = 0;

    bool BondFlag = GetDevBondFlag();
    if(BondFlag == false) return count;

    bool PpgEn = GetPpgEnableFlag();
    if(PpgEn == false) return count;

    count = HrGsR;

    u8 *pR = RFifoData;
    u8 *pW = WFifoData;

    /* 当前fifo正在写，用上一帧的数据 */
    //if(FifoWFlag == 1) goto __end;

    if(Wlen <= Rlen) goto __end;
    
    u32 r_len = count*6;
    memset(pR, 0, RFifoLen);

    //用新数据
    u32 fifo_len = Wlen - Rlen;//20

    //printf("fifo_len = %d\n", fifo_len);
    //printf("Wlen = %d, Rlen = %d\n", Wlen, Rlen);

    u32 len;
    if(fifo_len >= r_len)
        len = r_len;
    else
        len = fifo_len;

    if(Ridx + len <= FifoSize)
    {
        memcpy(pR, &pW[Ridx], len);
        Ridx += len;
    }else
    {
        u32 idx = 0;
        u32 remain = FifoSize - Ridx;
        if(remain > 0)
        {
            memcpy(&pR[idx], &pW[Ridx], remain);
            idx += remain;
        }

        Ridx = 0;
        remain = len - remain;
        if(remain > 0)
        {
            memcpy(&pR[idx], &pW[Ridx], remain);
            Ridx += remain;
        }
    }
    Rlen += len;

    if(len < r_len)
    {
        //补齐
        u32 idx = len;//>0
        u32 remain = r_len - len;//48 
        for(u32 i = 0; i < remain/6; i++)
        {
            memcpy(&pR[idx + 6*i], &pR[idx + 6*(i-1)], 6);
        }
    }

__end:
    u32 idx = 0;
    u8 fifo_data[6];
    s16 acc_data[3];
    for(u32 i = 0; i < count; i++)
    {
        memcpy(fifo_data, &pR[idx], 6);

        acc_data[0] = (s16)(((u16)fifo_data[1]<<8)|(fifo_data[0]));
        acc_data[1] = (s16)(((u16)fifo_data[3]<<8)|(fifo_data[2]));
        acc_data[2] = (s16)(((u16)fifo_data[5]<<8)|(fifo_data[4]));

        acc_x[i] = -acc_data[0];
        acc_y[i] = -acc_data[1];
        acc_z[i] = acc_data[2];

        idx += 6;
    }

    return count;
}
