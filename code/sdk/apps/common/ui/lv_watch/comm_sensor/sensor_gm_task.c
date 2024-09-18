#include "../lv_watch.h"

#define ONE_G (9.807f)
#define Cali_Vm_Mask (0x55bb)

GmCaliInfoPara_t Cali_Info;
static const GmCaliInfoPara_t InitInfo = {
    .cali_succ = false,
    .calipara = {0.0f},
};

/****地磁与加速度计参数配套*****/
#define FifoSize (GmGs_Fifo_WM*6*5)
static u32 Widx;
static u32 Ridx;
static u8 FifoWFlag;
static u8 WFifoData[FifoSize];

#define RMaxCnt (12)
#define RFifoLen (RMaxCnt*6)
static u8 RFifoData[RFifoLen];

static u32 AccIdx;
static s16 AccX[25];
static s16 AccY[25];
static s16 AccZ[25];

/****地磁算法传参*****/
float GmOutput[3] = {0.0f, 0.0f, 0.0f};
float GmRawData[3] = {0.0f, 0.0f, 0.0f};
float AccCorrect[3] = {0.0f, 0.0f, 0.0f};
float EulerValue[3] = {0.0f, 0.0f, 0.0f};
float GmOutputOffset[4] = {0.0f, 0.0f, 0.0f, 0.0f}; 

static int8_t QmcAccuracy;

#if GM_DATA_DEBUG
static u16 gm_idx;
static u16 gm_cnt;
float gm_data0[GM_LEN];
float gm_data1[GM_LEN];
float gm_data2[GM_LEN];
#endif

#if GM_DATA_BLE
    static u16 send_idx;
#endif

#if GM_DATA_BLE
static void GmRawDataSend(u16 s_idx, float *raw_data)
{
    u8 nfy_buf[Cmd_Pkt_Len];
    memset(nfy_buf, 0x00, Cmd_Pkt_Len);

    //printf("[0] = %f, [1] = %f, [2] = %f\n", raw_data[0], raw_data[1], raw_data[2]);

    int gm_0 = (int)(raw_data[0]*100000.0f+0.5f);
    int gm_1 = (int)(raw_data[1]*100000.0f+0.5f);
    int gm_2 = (int)(raw_data[2]*100000.0f+0.5f);
    //printf("s_idx = %d, gm_0 = %d, gm_1 = %d, gm_2 = %d\n", s_idx, gm_0, gm_1, gm_2);

    u8 idx = 0;
    nfy_buf[idx++] = 0x39;
    nfy_buf[idx++] = (s_idx>>8)&0xff;
    nfy_buf[idx++] = (s_idx)&0xff;
    nfy_buf[idx++] = (gm_0>>24)&0xff;
    nfy_buf[idx++] = (gm_0>>16)&0xff;
    nfy_buf[idx++] = (gm_0>>8)&0xff;
    nfy_buf[idx++] = (gm_0>>0)&0xff;
    nfy_buf[idx++] = (gm_1>>24)&0xff;
    nfy_buf[idx++] = (gm_1>>16)&0xff;
    nfy_buf[idx++] = (gm_1>>8)&0xff;
    nfy_buf[idx++] = (gm_1>>0)&0xff;
    nfy_buf[idx++] = (gm_2>>24)&0xff;
    nfy_buf[idx++] = (gm_2>>16)&0xff;
    nfy_buf[idx++] = (gm_2>>8)&0xff;
    nfy_buf[idx++] = (gm_2>>0)&0xff;

    u8 crc_idx = Cmd_Pkt_Len - 1;
    nfy_buf[crc_idx] = calc_crc(nfy_buf, crc_idx);

    //printf_buf(nfy_buf, Cmd_Pkt_Len);

    umeox_common_le_notify_data(nfy_buf, Cmd_Pkt_Len);

    return;
}
#endif

static void GmGsFifoParaInit(void)
{
    Widx = 0; Ridx = 0;
    memset(WFifoData, 0, FifoSize);
    memset(RFifoData, 0, RFifoLen);

#if GM_DATA_BLE
    send_idx = 0;
#endif

#if GM_DATA_DEBUG
    gm_idx = 0;
    gm_cnt = 0;
    // memset(gm_data0, 0, sizeof(gm_data0));
    // memset(gm_data1, 0, sizeof(gm_data1));
    // memset(gm_data2, 0, sizeof(gm_data2));
#endif
    return;
}

static void GmAccDataInit(void)
{
    AccIdx = 0;

    memset(AccX, 0, sizeof(AccX));
    memset(AccY, 0, sizeof(AccY));
    memset(AccZ, 0, sizeof(AccZ));

    return;
}

static u16 gm_timer;
static void TimerCb(void *priv)
{
    int GmMsg[2];
	GmMsg[0] = GmMsgProcess;
	PostGmTaskMsg(GmMsg, 1);

    return;
}

static void GmTimerCreate(void)
{
    if(gm_timer == 0)
        gm_timer = sys_hi_timer_add(NULL, TimerCb, 40);

    return;
}

static void GmTimerDestory(void)
{
    if(gm_timer)
        sys_hi_timer_del(gm_timer);
    gm_timer = 0;

    return;
}

static void EnableGmModuleHandle(void)
{
    GmAccDataInit();
    GmGsFifoParaInit();

    SetGmEnableFlag(true);
    qmc6309_enable();   
    GmTimerCreate();
     
    return;
}

static void DisableGmModuleHandle(void)
{
    GmTimerDestory();
    qmc6309_disable();
    SetGmEnableFlag(false);
    
    return;
}

static float qst_smc_evecs[3][3] = {
	{1.034f, 0.009f, 0.018f},
	{0.009f, 0.957f, 0.003f},
	{0.018f, 0.003f, 1.012f},
};

static void GmProcess(void)
{
    static u32 r_cnt = 0;
    static u8 CaliCnt = 0;

    if(AccIdx == 0)
        r_cnt = GmGsDataFifoRead(AccX, AccY, AccZ);//40*10 = 400ms

    //printf("r_cnt = %d\n", r_cnt);
    if(r_cnt == 0) return;
    
    u32 idx = AccIdx;
#if 0
    printf("%d:x = %d, y = %d, z = %d\n", idx, AccX[idx], AccY[idx], AccZ[idx]);
#endif
    
    if(AccX[idx] == 0 && AccY[idx] == 0 && AccZ[idx] == 0)
        return;

    AccCorrect[0] = (float)(AccX[idx]*ONE_G)/GetGsACCSsvt();
	AccCorrect[1] = (float)(AccY[idx]*ONE_G)/GetGsACCSsvt();
	AccCorrect[2] = (float)(AccZ[idx]*ONE_G)/GetGsACCSsvt();
    //printf("x = %f, y = %f, z = %f\n", AccCorrect[0], AccCorrect[1], AccCorrect[2]);
    
    AccIdx++;
    AccIdx %= r_cnt;

    qmc6309_read_mag_xyz(GmRawData);//地磁数据单件为：ut
    float raw_data[3];
    raw_data[0]=GmRawData[0]*qst_smc_evecs[0][0]+GmRawData[1]*qst_smc_evecs[0][1]+GmRawData[2]*qst_smc_evecs[0][2];
    raw_data[1]=GmRawData[0]*qst_smc_evecs[1][0]+GmRawData[1]*qst_smc_evecs[1][1]+GmRawData[2]*qst_smc_evecs[1][2];
    raw_data[2]=GmRawData[0]*qst_smc_evecs[2][0]+GmRawData[1]*qst_smc_evecs[2][1]+GmRawData[2]*qst_smc_evecs[2][2];
#if GM_DATA_BLE
    if(send_idx < 6000)
    {
        send_idx++;
        GmRawDataSend(send_idx, raw_data);
    }
#endif
    //printf("[0] = %f, [1] = %f, [2] = %f\n", raw_data[0], raw_data[1], raw_data[2]);
#if GM_DATA_DEBUG
    if(gm_cnt < GM_CNT)
    {
        if(gm_idx < GM_LEN)
        {
            gm_data0[gm_idx] = GmRawData[0];
            gm_data1[gm_idx] = GmRawData[1];
            gm_data2[gm_idx] = GmRawData[2];
            printf("gm_idx = %d\n", gm_idx);
            // printf("%d:[0] = %f, [1] = %f, [2] = %f\n", gm_idx, gm_data0[gm_idx], gm_data1[gm_idx], gm_data2[gm_idx]);
            gm_idx++;

            gm_idx %= GM_LEN;
            if(gm_idx == 0)
            {
                //写一个vmdata
                gm_cnt++;


            }
        }
    } 
#endif

    if((raw_data[0] == 0.0f) && (raw_data[1] == 0.0f) && (raw_data[2] == 0.0f))  // 处理全0情况。
    {
        raw_data[0]=1.0f; raw_data[1]=1.0f; raw_data[2]=1.0f;
    }

    fusion_docali(raw_data, AccCorrect, GmOutput, GmOutputOffset, EulerValue, &QmcAccuracy);
    s16 yaw = (s16)(EulerValue[0] + 0.5f);
	//printf("yaw = %d, QmcAccuracy = %d\n", yaw, QmcAccuracy); //EulerValue[0]:是地磁计算出来的yaw航向角;

    if(QmcAccuracy == 3)
    {
        //8字校准成功 
        CaliCnt = 0;
        SetGmYawAngle(yaw);
 
        bool *cali_succ = &(Cali_Info.cali_succ);
        if(*cali_succ == false)
        {
            *cali_succ = true;

            Cali_Info.calipara[0] = GmOutputOffset[0];
            Cali_Info.calipara[1] = GmOutputOffset[1];
            Cali_Info.calipara[2] = GmOutputOffset[2];
            Cali_Info.calipara[3] = GmOutputOffset[3];

            GmCaliInfoUpdate();

            printf("Update Cali Para succ\n");
        }
    }else if(QmcAccuracy == 0)
    {
        CaliCnt++;

        if(CaliCnt >= 100) 
        {
            CaliCnt = 0;

            bool *cali_succ = &(Cali_Info.cali_succ);
            *cali_succ = false;
            GmCaliInfoUpdate();
            
            printf("Recalibrate parameters\n");
        }
    }else
    {
        CaliCnt = 0;

        bool *cali_succ = &(Cali_Info.cali_succ);
        *cali_succ = false;
        GmCaliInfoUpdate();
    }

    return;
}

static void GmTask(void *p)
{
    int ret;
    int msg[8] = {0};

    GmGsFifoParaInit();
    GmAccDataInit();

    GmCaliInfoRead();

    qst_ical_init(Cali_Info.calipara);
	fusion_enable();

    /*开机先失能模块*/
    DisableGmModule();

    while(1)
    {
        ret = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg)); 

        if(ret == OS_TASKQ)
            GmTaskMsgHandle(msg, ARRAY_SIZE(msg));
    }
    
    return;
}

void GmTaskCreate(void)
{
    int err = task_create(GmTask, NULL, Gm_Task_Name);
    if(err) printf("Gm task create err!!!!!!!:%d \n", err);

    return;
}

void GmTaskMsgHandle(int *msg, u8 len)
{
    if(!msg || len == 0)
        return;

    int cmd = msg[0];

    switch(cmd)
    {
        case GmMsgProcess:
            GmProcess();
            break;
        
        case GmMsgEnable:
            EnableGmModuleHandle();
            break;

        case GmMsgDisable:
            DisableGmModuleHandle(); 
            break;

        default:
            printf("*************Gm msg not found\n");
            break;
    }

    return;
}

int PostGmTaskMsg(int *msg, u8 len)
{
    int err = 0;
    int count = 0;

    if(!msg || len == 0)
        return -1;

__retry:
    err = os_taskq_post_type(Gm_Task_Name, msg[0], \
        len - 1, &msg[1]);

    if(cpu_in_irq() || cpu_irq_disabled())
        return err;

    if(err) 
    {
        if(!strcmp(os_current_task(), Gm_Task_Name)) 
            return err;

        if(count > 20)
            return -1;
        
        count++;
        os_time_dly(1);
        goto __retry;
    }

    return err;
}

/***********地磁航向角**************/
static s16 YawAngle;
s16 GetGmYawAngle(void)
{
    return YawAngle;
}

void SetGmYawAngle(s16 yaw)
{
    YawAngle = yaw;
    return;
}

/***********地磁使能标志**************/
static bool GmEn;
bool GetGmEnableFlag(void)
{
    return GmEn;
}

void SetGmEnableFlag(bool f)
{
    GmEn = f;
    return;
}

/***********地磁校准成功**************/
bool GetGmCaliSucc(void)
{
    return Cali_Info.cali_succ;
}

void SetGmCaliSucc(bool f)
{
    Cali_Info.cali_succ = f;

    return;
}

/***********地磁模块启动/停止**************/
void EnableGmModule(void)
{
    bool gm_en = GetGmEnableFlag();
    if(gm_en == true) return;

    int GmMsg[2];
	GmMsg[0] = GmMsgEnable;
	PostGmTaskMsg(GmMsg, 1); 

    return;
}

void DisableGmModule(void)
{
    bool gm_en = GetGmEnableFlag();
    if(gm_en == false) return;

    int GmMsg[2];
	GmMsg[0] = GmMsgDisable;
	PostGmTaskMsg(GmMsg, 1); 

    return;
}

void GmGsDataFifoWrite(u8 *w_buf, u32 w_len)
{
    bool BondFlag = GetDevBondFlag();
    if(BondFlag == false) return;

    // bool gm_en = GetGmEnableFlag();
    // if(gm_en == false) return;

    FifoWFlag = 1;
    u8 *pW = WFifoData;
    Ridx = Widx;
    if(Widx + w_len <= FifoSize)
    {
        memcpy(&pW[Widx], w_buf, w_len);
        Widx += w_len;
    }else
    {
        u32 idx = 0;
        u32 remain = FifoSize - Widx;
        if(remain > 0)
        {
            memcpy(&pW[Widx], &w_buf[idx], remain);
            idx += remain;
        }
        
        Widx = 0;
        remain = w_len - remain;//20
        if(remain > 0)
        {   
            memcpy(&pW[Widx], &w_buf[idx], remain);
            Widx += remain;
        }
    }
    FifoWFlag = 0;
    return;
}

u32 GmGsDataFifoRead(s16 *acc_x, s16 *acc_y, s16 *acc_z)
{
    u32 count = 0;

    bool BondFlag = GetDevBondFlag();
    if(BondFlag == false) return count;

    bool gm_en = GetGmEnableFlag();
    if(gm_en == false) return count;

    u8 *pR = RFifoData;
    u8 *pW = WFifoData;

    count = GmGs_Fifo_WM;

    //if(FifoWFlag == 1) goto __end;

    u32 len = count*6;
    memset(pR, 0, RFifoLen);

    if(Ridx + len <= FifoSize)
    {
        memcpy(pR, &pW[Ridx], len);
    }else
    {
        u32 idx = 0;
        u32 remain = FifoSize - Ridx;
        if(remain > 0)
        {
            memcpy(&pR[idx], &pW[Ridx], remain);
            idx += remain;
        }

        remain = len - remain;
        if(remain > 0)
        {
            memcpy(&pR[idx], &pW[0], remain);
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

void GmCaliInfoRead(void)
{
    int vm_op_len = sizeof(GmCaliInfoPara_t);
    int ret = syscfg_read(CFG_GM_CALIPARA_INFO, &Cali_Info, vm_op_len);
    if(ret != vm_op_len || Cali_Info.vm_mask != Cali_Vm_Mask)
        GmCaliInfoReset();
    return;
}

void GmCaliInfoWrite(void)
{
    int vm_op_len = sizeof(GmCaliInfoPara_t);
    for(u8 i = 0; i < 3; i++)
    {
        int ret = syscfg_write(CFG_GM_CALIPARA_INFO, &Cali_Info, vm_op_len);
        if(ret == vm_op_len)
            break;
    }
    return;
}

void GmCaliInfoReset(void)
{
    int vm_op_len = sizeof(GmCaliInfoPara_t);
    memcpy(&Cali_Info, &InitInfo, vm_op_len);     
    Cali_Info.vm_mask = Cali_Vm_Mask;
    GmCaliInfoUpdate();

    return;
}

void GmCaliInfoUpdate(void)
{
    GmCaliInfoWrite();
    return;
}
