#include "../lv_watch.h"

#define VM_MASK (0x55aa)

SlpPara_t SlpPara;

#define __this_module (&SlpPara)
#define __this_module_size (sizeof(SlpPara_t))
static const SlpPara_t init = {
    .valid = false, 
    .slp_dur = 0, 
    .d_slp_dur = 0, .l_slp_dur = 0, .r_slp_dur = 0, .wake_dur = 0,
    .slp_start_ts = 0,.slp_end_ts = 0,
};

struct SleepSummaryInput slp_in;
struct SleepSummaryOutput slp_out;

/* true：睡眠启动 false:睡眠关闭*/
static bool slp_en;
bool GetSleepEnState(void)
{
    return slp_en;
}

void SetSleepEnState(bool en)
{
    slp_en = en;
    return;
}

/* true：入睡成功 false：醒来 */
static bool FallAsleep;
bool GetFallAsleepFlag(void)
{
    return FallAsleep;
}

void SetFallAsleepFlag(bool f)
{
    FallAsleep = f;
    return;
}

/* 睡眠ppg使能状态 */
static u8 SlpPpgEn;
u8 GetSlpPpgEnState(void)
{
    return SlpPpgEn;
}

void SetSlpPpgEnState(u8 en)
{
    SlpPpgEn = en;
    return;
}

static u8 SlpPpgWear;
u8 GetSlpPpgWearState(void)
{
    return SlpPpgWear;
}

void SetSlpPpgWearState(u8 state)
{
    /* 如果不是睡眠ppg开的，不管 */
    u8 slp_ppg_en = GetSlpPpgEnState();
    if(slp_ppg_en == 0) return;

    SlpPpgWear = state;
    return;
}

static u32 Widx;
static u32 Wlen;
#define Data_Len (25)
#define FifoLen (Data_Len*2)
static float w_data[FifoLen];
void SlpHrRawDataInit(void)
{
#if 0
    WIdx = 0; RIdx = 0;
    WLen = 0; RLen = 0;
    for(u16 i = 0; i < Data_Len; i++)
        r_raw_data[i] = 0.0f;
#else
    Widx = 0; Wlen = 0;
#endif
    return;
}

u8 GetSlpHrRawDataLen(void)
{
    u8 len;
    u8 slp_ppg_en = GetSlpPpgEnState();
    if(slp_ppg_en == 0)
        len = 0;
    else
        len = Data_Len;

    return len;
}

float *GetSlpHrRawData(void)
{
#if 0
    static float *pData;
    pData = r_raw_data;

    //if(FifoWFlag == 1) goto __end;

    if(WLen <= RLen) goto __end;

    //printf("WLen = %d, RLen = %d\n", WLen, RLen);

    u8 rlen = WLen - RLen;
    if(rlen >= Data_Len)
    {
        if(RIdx + Data_Len <= FifoLen)
        {
            memcpy(pData, &w_raw_data[RIdx], Data_Len*sizeof(float));
            RIdx += Data_Len;
        }else
        {
            u8 idx = 0;
            u8 remain = FifoLen - RIdx;
            if(remain > 0)
            {
                memcpy(&pData[idx], &w_raw_data[RIdx], remain*sizeof(float));
                idx += remain;
            }
            RIdx = 0;
            remain = Data_Len - remain;
            if(remain > 0)
            {
                memcpy(&pData[idx], &w_raw_data[RIdx], remain*sizeof(float));
                RIdx += remain;
            }
        }

        RLen += Data_Len;
    }else
    {
        //如果说不够25条，最后一条补
        if(RIdx + rlen <= FifoLen)
        {
            memcpy(pData, &w_raw_data[RIdx], rlen*sizeof(float));
            RIdx += rlen;

            u8 idx = rlen;
            u8 remain = Data_Len - rlen;//1
            for(u8 i = 0; i < remain; i++)
            {
                pData[idx + i] = pData[idx + i - 1];
            }
        }else
        {
            u8 idx = 0;
            u8 remain = FifoLen - RIdx;
            if(remain > 0)
            {
                memcpy(&pData[idx], &w_raw_data[RIdx], remain*sizeof(float));
                idx += remain;
            }
            RIdx = 0;
            remain = rlen - remain;
            if(remain > 0)
            {
                memcpy(&pData[idx], &w_raw_data[RIdx], remain*sizeof(float));
                RIdx += remain;
                idx += remain;
            }

            remain = Data_Len - rlen;
            for(u8 i = 0; i < remain; i++)
            {
                pData[idx + i] = pData[idx + i - 1];
            }
        }

        RLen += rlen;
    }
    
    __end:
    return pData;
#else
    static float r_data[Data_Len];

    if(Wlen == 0) goto __end;
    printf("Wlen = %d\n", Wlen);

    memset(r_data, 0, sizeof(r_data));
    u32 Rlen = Wlen;
    Widx = 0; Wlen = 0;
    u32 count = Rlen;
    if(count > Data_Len)
        count = Data_Len;
    memcpy(r_data, w_data, count*sizeof(float));//20

    if(count < Data_Len)
    {
        u8 idx = count;
        u8 remain = Data_Len - count;//5
        for(u8 i = 0; i < remain; i++)
        {
            r_data[idx + i] = r_data[idx + i - 1];
        }
    }
    // for(u8 i = 0; i < Data_Len; i++)
    // {
    //     printf("r_data[%d] = %f", i, r_data[i]);
    // }
__end:
    return r_data;
#endif
}

void SetSlpHrRawData(int32_t *buf, u16 len)
{
    u8 slp_ppg_en = GetSlpPpgEnState();
    if(slp_ppg_en == 0) return;

    //FifoWFlag = 1;

    u8 idx = 0;
    if(Widx + len <= FifoLen)
    {
        for(u16 i = 0; i < len; i++)
        {
            w_data[Widx+i] = buf[idx + i]*1.0f;
        }
        Widx += len;
    }else
    {
        u8 remain = FifoLen - Widx;
        for(u16 i = 0; i < remain; i++)
        {
            w_data[Widx+i] = buf[idx + i]*1.0f;
        }
        idx += remain;

        Widx = 0;
        remain = len - remain;
        for(u16 i = 0; i < remain; i++)
        {
            w_data[Widx+i] = buf[idx + i]*1.0f;
        }
        Widx += remain;
    }
    Wlen += len;
    //printf("Wlen = %d\n", Wlen);
    //FifoWFlag = 0;
    // for(u8 i = 0; i < Wlen; i++)
    // {
    //     printf("w_data[%d] = %f", i, w_data[i]);
    // }
    return;
}

//false 没过期  true 过期
static bool SleepVmDataIsPast(u32 timestamp)
{
    struct sys_time time;
    GetUtcTime(&time);
    u32 cur_timestamp = UtcTimeToSec(&time);
    if(cur_timestamp < timestamp) goto __end;

    if(cur_timestamp - timestamp < SEC_PER_DAY) 
        return false;

__end:
    return true;
}

static bool GetSleepData(void)
{
    bool ret = false;

    u8 num = VmSleepItemNum();
    if(num == 0) goto __end;

    u8 idx = num - 1;
    bool r_ret = VmSleepCtxByIdx(idx);
    if(r_ret == false) goto __end;

    ret = true;

__end:
    return ret;
}

void PowerOnSetSleepData(void)
{
    printf("%s:%d\n", __func__, sizeof(vm_sleep_ctx_t));

    u8 num = VmSleepItemNum();
    printf("%s:sleep num = %d\n", __func__, num);

    /*读取vm的最新一条数据*/
    bool data_ret = GetSleepData();
    if(data_ret == false) return;

    /*判断vm的最新数据是否已经过期*/
    bool IsPast = SleepVmDataIsPast(r_sleep.timestamp);
    printf("%s:IsPast = %d\n", __func__, IsPast);
    if(IsPast == true) SlpDataVmReset();

    return;
}

/* 设置算法自动监测标志位 */
static void SetSleepAutoDetEnFlag(u8 en)//0 1
{
    en = !en;
    SleepConfig slp_cfg;
    getSleepConfig(0, &slp_cfg);
    if(slp_cfg.disableAutoDetection == en)
        return;
    slp_cfg.disableAutoDetection = en;
    int16_t cfg = setSleepConfig(&slp_cfg);
    if(cfg == 0) printf("setSleepConfig success\n");
    // printf("minNapLength = %d\n", slp_cfg.minNapLength);
    // printf("maxGapLength = %d\n", slp_cfg.maxGapLength);
    // printf("disablePeriodFinetuning = %d\n", slp_cfg.disablePeriodFinetuning);
    // printf("disableAutoDetection = %d\n", slp_cfg.disableAutoDetection);
    // printf("longSleepPeriod[0] = %d\n", slp_cfg.longSleepPeriod[0]);
    // printf("longSleepPeriod[1] = %d\n", slp_cfg.longSleepPeriod[1]);

    return;
}

// #define Slp_Max_Sum (3)
// static u8 slp_sum_num;
static u8 SlpSecNum;
static SlpSecCtx_t pslp_ctx[Slp_Max_Sec];
// static SlpSecCtx_t awake_inv[Slp_Max_Sum-1];

#if 0
static void ClearSlpSec(void)
{
    slp_sum_num = 0;
    memset(slp_sec_num, 0, Slp_Max_Sum);
    for(u8 i = 0; i < Slp_Max_Sum; i++)
    {
        if(pslp_ctx[i] != NULL)
            free(pslp_ctx[i]);
        pslp_ctx[i] = NULL;
    }

    //memset(awake_inv, 0, sizeof())
    // memset(slp_inv_st, 0, sizeof(slp_inv_st));
    // memset(slp_inv_et, 0, sizeof(slp_inv_et));

    return;
}
#endif

static void SlpSecInit(void)
{
    SlpSecNum = 0;
    memset(pslp_ctx, 0, sizeof(SlpSecCtx_t)*Slp_Max_Sec);
    return;
}

static void GetSecSlpSummary(struct SleepSummaryInput *in, struct SleepSummaryOutput *out) 
{
    getEmbeddedSleepSummary(in, out);

    printf("------------------>sleep type = %d\n", out->type);
    if(out->type == 1){
        SlpSecInit();
        SlpDataVmReset();

        //长睡
        u8 idx = 0;
        int8_t last_stage = -1;//上一个状态
        int32_t numEpochs = out->numEpochs;
        for(int32_t i = 0; i < numEpochs; i++)
        {
            if(last_stage != out->stages[i])
            {
                if(idx >= Slp_Max_Sec)
                    break;

                last_stage = out->stages[i];
                pslp_ctx[idx].stage = last_stage;
                pslp_ctx[idx].start_ts = out->startTS + i*30;
                pslp_ctx[idx].end_ts = out->startTS + (i+1)*30;
                idx += 1;
            }else
            {
                if(idx == 0) continue;

                pslp_ctx[idx - 1].end_ts = out->startTS + (i+1)*30;
            }
        }
        SlpSecNum = idx;

        /* 开始时间为第一段start_ts */
        u32 start_ts = pslp_ctx[0].start_ts;
        /* 结束时间为最后一段的end_ts */
        u8 lidx;
        if(idx > 0) lidx = idx-1;
        else lidx = 0;
        u32 end_ts = pslp_ctx[lidx].end_ts;
        printf("start_ts = %d, end_ts = %d\n", start_ts, end_ts);

        /* 用户展示数据 */
        __this_module->valid = true;
        __this_module->slp_start_ts = start_ts;
        __this_module->slp_end_ts = end_ts;
        __this_module->slp_dur = (end_ts - start_ts)/60;
        printf("__this_module slp_dur = %d\n", __this_module->slp_dur);
        for(u8 i = 0; i < idx; i++)
        {
            u8 stage = pslp_ctx[i].stage;
            u32 dur = pslp_ctx[i].end_ts - pslp_ctx[i].start_ts;
            printf("__this_module i = %d, stage = %d:dur = %d\n", i, stage, dur);
            if(stage == slp_awake)
                __this_module->wake_dur += dur;
            else if(stage == slp_rem)
                __this_module->r_slp_dur += dur;
            else if(stage == slp_light)
                __this_module->l_slp_dur += dur;
            else if(stage == slp_deep)
                __this_module->d_slp_dur += dur;
        }
        
        /*单位分钟*/
        __this_module->wake_dur /= 60;
        __this_module->r_slp_dur /= 60;
        __this_module->l_slp_dur /= 60;
        __this_module->d_slp_dur /= 60;
        printf("__this_module wake_dur = %d, r_slp_dur = %d, l_slp_dur = %d, d_slp_dur = %d\n", __this_module->wake_dur, \
            __this_module->r_slp_dur, __this_module->l_slp_dur, __this_module->d_slp_dur);
        SlpDataVmWrite();
        // __this_module->valid = true;
        // __this_module->slp_start_ts = out->startTS;
        // __this_module->slp_end_ts = out->endTS;
        // __this_module->slp_dur = (u32)(out->sleepPeriod + 0.5f);
        // __this_module->d_slp_dur = (u32)(out->deepNumMinutes + 0.5f);
        // __this_module->l_slp_dur = (u32)(out->lightNumMinutes + 0.5f);
        // __this_module->r_slp_dur = (u32)(out->remNumMinutes + 0.5f);
        // __this_module->wake_dur = (u32)(out->wakeNumMinutes + 0.5f);
        //SlpDataVmWrite();
#if 0
        memset(pslp_ctx, 0, sizeof(SlpSecCtx_t)*Slp_Max_Sec);

        //长睡
        u8 idx = 0;
        int8_t last_stage = -1;//上一个状态
        int32_t numEpochs = out->numEpochs;
        for(int32_t i = 0; i < numEpochs; i++)
        {
            if(last_stage != out->stages[i])
            {
                if(idx >= Slp_Max_Sec)
                    break;

                last_stage = out->stages[i];
                pslp_ctx[idx].stage = last_stage;
                pslp_ctx[idx].start_ts = out->startTS + i*30;
                pslp_ctx[idx].end_ts = out->startTS + (i+1)*30;
                idx += 1;
            }else
            {
                if(idx == 0) continue;

                pslp_ctx[idx - 1].end_ts = out->startTS + (i+1)*30;
            }
        }

        /* 开始时间为第一段start_ts */
        u32 start_ts = pslp_ctx[0].start_ts;
        /* 结束时间为最后一段的end_ts */
        u8 lidx;
        if(idx > 0) lidx = idx-1;
        else lidx = 0;
        u32 end_ts = pslp_ctx[0].end_ts;
        printf("start_ts = %d, end_ts = %d\n", start_ts, end_ts);

        /* 用户展示数据 */
        __this_module->valid = true;
        __this_module->slp_start_ts = start_ts;
        __this_module->slp_end_ts = end_ts;
        __this_module->slp_dur = (end_ts - start_ts)/60;
        printf("__this_module slp_dur = %d\n", __this_module->slp_dur);
        for(u8 i = 0; i < idx; i++)
        {
            u8 stage = pslp_ctx[i].stage;
            u32 dur = pslp_ctx[i].end_ts - pslp_ctx[i].start_ts;
            printf("__this_module i = %d, stage = %d:dur = %d\n", i, stage, dur);
            if(stage == slp_awake)
                __this_module->wake_dur += dur;
            else if(stage == slp_rem)
                __this_module->r_slp_dur += dur;
            else if(stage == slp_light)
                __this_module->l_slp_dur += dur;
            else if(stage == slp_deep)
                __this_module->d_slp_dur += dur;
        }
        
        /*单位分钟*/
        __this_module->wake_dur /= 60;
        __this_module->r_slp_dur /= 60;
        __this_module->l_slp_dur /= 60;
        __this_module->d_slp_dur /= 60;
        printf("__this_module wake_dur = %d, r_slp_dur = %d, l_slp_dur = %d, d_slp_dur = %d\n", __this_module->wake_dur, \
            __this_module->r_slp_dur, __this_module->l_slp_dur, __this_module->d_slp_dur);
        SlpDataVmWrite();
#endif
    }else if(out->type == 2){
        //短睡
        // __this_module->valid = true;
        // __this_module->slp_start_ts = out->startTS;
        // __this_module->slp_end_ts = out->endTS;
        // __this_module->slp_dur = 0;
    }


#if 0
    getEmbeddedSleepSummary(in, out);

    printf("sleep type = %d\n", out->type);
    /* 短睡不做处理 */
    if(out->type == 2) return;

    if(slp_sum_num >= Slp_Max_Sum) return;

    /* 长睡 */
    if(out->type == 1)
    {
        u8 ssn = slp_sum_num;
        pslp_ctx[ssn] = (SlpSecCtx_t *)malloc(sizeof(SlpSecCtx_t)*Slp_Max_Sec);
        if(pslp_ctx[ssn] == NULL) return;

        u8 idx = 0;
        int8_t last_stage = -1;//上一个状态
        int32_t numEpochs = out->numEpochs;
        for(int32_t i = 0; i < numEpochs; i++)
        {
            if(last_stage != out->stages[i])
            {
                if(idx >= Slp_Max_Sec)
                    break;

                last_stage = out->stages[i];
                pslp_ctx[ssn][idx].stage = last_stage;
                pslp_ctx[ssn][idx].start_ts = out->startTS + i*30;
                pslp_ctx[ssn][idx].end_ts = out->startTS + (i+1)*30;
                idx += 1;
            }else
            {
                if(idx == 0) continue;

                pslp_ctx[ssn][idx - 1].end_ts = out->startTS + (i+1)*30;
            }
        }

        slp_sec_num[ssn] = idx;
        slp_sum_num += 1;

        for(u8 i = 0; i < idx; i++)
        {
            printf("%d:st = %d, se = %d, stage = %d\n", i, \
                pslp_ctx[ssn][i].start_ts, pslp_ctx[ssn][i].end_ts, pslp_ctx[ssn][i].stage);
        }
    }
#endif

    return;
}

#if 1
static void GetDaySlpSummary(void)
{
    if(SlpSecNum == 0) return;
    u8 sec_num = SlpSecNum;

    for(u8 i = 0; i < sec_num; i++)
    {
        w_sleep.ctx[i].stage = pslp_ctx[i].stage;
        w_sleep.ctx[i].start_ts = pslp_ctx[i].start_ts;
        w_sleep.ctx[i].end_ts = pslp_ctx[i].end_ts;
    }

    for(u8 i = 0; i < sec_num; i++)
    {
        printf("w_sleep %d: stage = %d, start_ts = %d, end_ts = %d\n", i, \
            w_sleep.ctx[i].stage, w_sleep.ctx[i].start_ts, w_sleep.ctx[i].end_ts);
    }

    /* 开始时间为第一段start_ts */
    u32 start_ts = w_sleep.ctx[0].start_ts;
    /* 结束时间为最后一段的end_ts */
    u8 lidx;
    if(sec_num > 0) lidx = sec_num-1;
    else lidx = 0;
    u32 end_ts = w_sleep.ctx[lidx].end_ts;

    w_sleep.total_start_ts = start_ts;
    w_sleep.total_end_ts = end_ts;
    w_sleep.SecNum = sec_num;
    w_sleep.check_code = Nor_Vm_Check_Code;
    int msg[2];
    msg[0] = ui_msg_nor_sleep_write;
    post_ui_msg(msg, 1);
    printf("w_sleep start_ts = %d, end_ts = %d\n", start_ts, end_ts);
#if 0
    //短睡不做处理
    struct SleepSummaryOutput *out = &slp_out;
    if(out->type == 2) return;

    u8 idx = 0;
    int8_t last_stage = -1;//上一个状态
    int32_t numEpochs = out->numEpochs;
    for(int32_t i = 0; i < numEpochs; i++)
    {
        if(last_stage != out->stages[i])
        {
            if(idx >= Slp_Max_Sec)
                break;

            last_stage = out->stages[i];
            w_sleep.ctx[idx].stage = last_stage;
            w_sleep.ctx[idx].start_ts = out->startTS + i*30;
            w_sleep.ctx[idx].end_ts = out->startTS + (i+1)*30;
            idx += 1;
        }else
        {
            if(idx == 0) continue;

            w_sleep.ctx[idx - 1].end_ts = out->startTS + (i+1)*30;
        }
    }

    for(u8 i = 0; i < idx; i++)
    {
        printf("w_sleep %d: stage = %d, start_ts = %d, end_ts = %d\n", i, \
            w_sleep.ctx[i].stage, w_sleep.ctx[i].start_ts, w_sleep.ctx[i].end_ts);
    }

    w_sleep.total_start_ts = out->startTS;
    w_sleep.total_end_ts = out->endTS;
    w_sleep.SecNum = idx;
    w_sleep.check_code = Nor_Vm_Check_Code;
    int msg[2];
    msg[0] = ui_msg_nor_sleep_write;
    post_ui_msg(msg, 1);
    printf("w_sleep: st = %d, et = %d, num = %d\n", out->startTS, out->endTS, idx);
#endif

#if 0
    if(slp_sum_num == 0) 
        return;

    u8 ssn = slp_sum_num;
    printf("____________%s:ssn = %d\n", __func__, ssn);
    if(ssn >= 2)
    {
        //两段睡眠以上
        u8 cur_idx;
        u8 next_idx;
        u8 sec_num;

        u8 inv_idx = 0;
        for(u8 i = 0; i < ssn; i++)
        {
            cur_idx = i;
            next_idx = i+1;
            sec_num = slp_sec_num[cur_idx];
            awake_inv[inv_idx].stage = slp_awake;
            awake_inv[inv_idx].start_ts = pslp_ctx[cur_idx][sec_num-1].end_ts;//间隔中间开始：上一段的结束 下一段的开始
            awake_inv[inv_idx].end_ts = pslp_ctx[next_idx][0].start_ts;
            printf("___%d  awake stage = %d, start_ts = %d, end_ts = %d\n", inv_idx, awake_inv[inv_idx].stage, \
                awake_inv[inv_idx].start_ts, awake_inv[inv_idx].end_ts);
            inv_idx++;
            if(inv_idx >= ssn-1) 
                break;
        }
    }

    u8 idx = 0;
    for(u8 i = 0; i < ssn; i++)
    {
        u8 sec_num = slp_sec_num[i];
        for(u8 j = 0; j < sec_num; j++)
        {
            if(idx >= Slp_Max_Sec) 
                goto __ctx_end;

            w_sleep.ctx[idx].stage = pslp_ctx[i][j].stage;
            w_sleep.ctx[idx].start_ts = pslp_ctx[i][j].start_ts;
            w_sleep.ctx[idx].end_ts = pslp_ctx[i][j].end_ts;
            idx++;
        }

        //中间插一段睡眠清醒
        if(i < ssn - 1)
        {
            if(idx >= Slp_Max_Sec) 
                goto __ctx_end;

            w_sleep.ctx[idx].stage = awake_inv[i].stage;
            w_sleep.ctx[idx].start_ts = awake_inv[i].start_ts;
            w_sleep.ctx[idx].end_ts = awake_inv[i].end_ts;
            idx++;
        }
    }

    for(u8 i = 0; i < idx; i++)
    {
        printf("sleep %d: stage = %d, start_ts = %d, end_ts = %d\n", i, \
            w_sleep.ctx[i].stage, w_sleep.ctx[i].start_ts, w_sleep.ctx[i].end_ts);
    }

__ctx_end:
    /* 开始时间为第一段start_ts */
    u32 start_ts = w_sleep.ctx[0].start_ts;
    /* 结束时间为最后一段的end_ts */
    u8 lidx;
    if(idx > 0) lidx = idx-1;
    else lidx = 0;
    u32 end_ts = w_sleep.ctx[lidx].end_ts;

    w_sleep.total_start_ts = start_ts;
    w_sleep.total_end_ts = end_ts;
    w_sleep.SecNum = idx;
    w_sleep.check_code = Nor_Vm_Check_Code;
    int msg[2];
    msg[0] = ui_msg_nor_sleep_write;
    post_ui_msg(msg, 1);
    printf("w_sleep start_ts = %d, end_ts = %d\n", start_ts, end_ts);

    /* 用户展示数据 */
    __this_module->valid = true;
    __this_module->slp_start_ts = start_ts;
    __this_module->slp_end_ts = end_ts;
    __this_module->slp_dur = (end_ts - start_ts)/60;
    printf("__this_module slp_dur = %d\n", __this_module->slp_dur);
    for(u8 i = 0; i < idx; i++)
    {
        u8 stage = w_sleep.ctx[i].stage;
        u32 dur = w_sleep.ctx[i].end_ts - w_sleep.ctx[i].start_ts;
        printf("__this_module %d:dur = %d\n", i, dur);
        if(stage == slp_awake)
            __this_module->wake_dur += dur;
        else if(stage == slp_rem)
            __this_module->r_slp_dur += dur;
        else if(stage == slp_light)
            __this_module->l_slp_dur += dur;
        else if(stage == slp_deep)
            __this_module->d_slp_dur += dur;
    }
    
    /*单位分钟*/
    __this_module->wake_dur /= 60;
    __this_module->r_slp_dur /= 60;
    __this_module->l_slp_dur /= 60;
    __this_module->d_slp_dur /= 60;
    printf("__this_module wake_dur = %d, r_slp_dur = %d, l_slp_dur = %d, d_slp_dur = %d\n", __this_module->wake_dur, \
        __this_module->r_slp_dur, __this_module->l_slp_dur, __this_module->d_slp_dur);
    SlpDataVmWrite();

    ClearSlpSec();
#endif
    return;
}
#endif

void VmFlashSleepCtxWrite(void)
{
    if(w_sleep.check_code != 0)
        VmSleepCtxFlashSave(&w_sleep);

    memset(&w_sleep, 0, sizeof(vm_sleep_ctx_t));

    return;
}

/* 根据睡眠反馈，打开ppg监测 todo */
void SleepPpgSensorOnOff(struct sys_time *ptime, s8 onoff)
{
    /* 没有监测到入睡，ppg不做处理 */
    // bool fall_asleep = GetFallAsleepFlag();
    // if(fall_asleep == false) return;

    //printf("______________________onoff = %d\n", onoff);

    /* 入睡阶段，心率由睡眠算法去接管 */
    if(onoff == 1)
    {
        u8 work = GetPpgWorkType();
        if(work == PpgWorkBo)
        {
            if(GetPpgMode() == PpgModeAuto)
                SetAutoBoRestart();
            DisablePpgModule();
        }else if(work == PpgWorkHr)
        {
            u8 slp_ppg_en = GetSlpPpgEnState();
            if(slp_ppg_en == 1) return;

            DisablePpgModule();
        }
            
        /* 启动睡眠心率 */
        SlpHrRawDataInit();
        SetSlpPpgEnState(1);
        EnablePpgModule(PpgWorkHr, PpgModeAuto);
        printf("------------------------------------------------>sleep ppg enable\n"); 
    }else
    {
        /* 看当前ppg工作类型 */  
        u8 slp_ppg_en = GetSlpPpgEnState();
        if(slp_ppg_en == 1)
        {
            /* 关闭睡眠心率 */
            DisablePpgModule();
            SetSlpPpgEnState(0);
            SlpHrRawDataInit();
            printf("------------------------------------------------>sleep ppg disable\n"); 
        }
    }

    return;
}

static 

/* 根据睡眠反馈，设置状态 */
void SleepStatusOutHandle(struct sys_time *ptime, u8 status)
{
    //printf("status = 0x%x\n", status);
#if 0//Sleep_Debug
    static u8 slp_sec = 0;
#endif

    if(status & 0x02) 
    {
        /* 监测入睡成功 */
        SetFallAsleepFlag(true);

        printf("______________________________________fall asleep");
    }else if(status & 0x04)
    {
        printf("______________________________________awake status = 0x%x", status);

        /* 监测出睡成功 */
        SetFallAsleepFlag(false);

#if !Sleep_Debug
        u8 slp_ppg_en = GetSlpPpgEnState();
        if(slp_ppg_en == 1)
        {
            DisablePpgModule();
            SetSlpPpgEnState(0);
        }
#endif

        if(status & 0x08)
            GetSecSlpSummary(&slp_in, &slp_out);

#if 0//Sleep_Debug 
        slp_sec += 1;
        if(slp_sec % 2 == 1)
            return;
#else
        /* 出睡先判断当前时间，如果说还在监测时间范围内，继续开始下一段监测。如果不在，认为整段睡眠结束 */
        u8 hour = ptime->hour;
        if(hour >= SlpStartH || hour < SlpEndH)
            return;
#endif

        printf("______________________________________sleep end status = 0x%x", status);
        /* 结束整段睡眠，统计一天的数据 */
        SetSleepEnState(false);
        SetSleepAutoDetEnFlag(0);
        GetDaySlpSummary();
    }

    return;
}

/* 手动结束睡眠处理，比如：充电 */
void ManualEndSleepHandle(void)
{
    bool slp_en = GetSleepEnState();
    if(slp_en == false) return;

    u8 slp_ppg_en = GetSlpPpgEnState();
    if(slp_ppg_en == 1)
    {
        DisablePpgModule();
        SetSlpPpgEnState(0);
    }

    struct SleepOutput so;
    endSleepPeriod(0, &so);
    if(so.sleepPeriodStatusOut & 0x08) 
        GetSecSlpSummary(&slp_in, &slp_out);

    SetFallAsleepFlag(false);
    SetSleepAutoDetEnFlag(0);
    GetDaySlpSummary();
    SetSleepEnState(false);

    return;
}

void SleepUtcMinProcess(struct sys_time *ptime)
{
    bool BondFlag = GetDevBondFlag();
    if(BondFlag == false) return;

    u8 hour = ptime->hour;
    u8 minute = ptime->min;
    if(hour >= SlpStartH || hour < SlpEndH)
    {
        /* 充电中，不启动睡眠监测 */
        u8 charge_state = GetChargeState();
        if(charge_state == 1) return;

        /* 防止上电进入睡眠 */
        if(ptime->year <= Sys_Def_Year) 
            return;

        /* 20:00 ~ 8:00 */
        bool slp_en = GetSleepEnState(); 
        if(slp_en == true) return;

        SlpSecInit();
        SlpDataVmReset();

        /* 时间点刻印在20点 */
        memset(&w_sleep, 0, sizeof(vm_sleep_ctx_t));
        struct sys_time time;
        memcpy(&time, ptime, sizeof(struct sys_time));
        time.hour = SlpStartH; time.min = 0; time.sec = 0;
        if(hour >= SlpStartH)
            w_sleep.timestamp = UtcTimeToSec(&time);
        else
            w_sleep.timestamp = UtcTimeToSec(&time) - SEC_PER_DAY;

        SetSleepEnState(true);
        SetFallAsleepFlag(false);
        SetSleepAutoDetEnFlag(1);
        printf("------------------------------------------------->sleep start\n");
    }else
    {
        /* 早上8点后，监测到睡眠清醒，认为结束，停止整个睡眠段 */
        bool fall_alseep = GetFallAsleepFlag();
        if(fall_alseep == true) return;

        bool slp_en = GetSleepEnState();
        if(slp_en == false) return;

        SetSleepAutoDetEnFlag(0);
        GetDaySlpSummary();
        SetSleepEnState(false);
        printf("------------------------------------------------->sleep end\n");
    }

    return;
}

void SlpDataVmRead(void)
{
    int ret = syscfg_read(CFG_SLP_PARA_INFO, __this_module, __this_module_size);
    if(ret != __this_module_size || __this_module->vm_mask != VM_MASK)
        SlpDataVmReset();

    return;
}

void SlpDataVmWrite(void)
{
    for(u8 i = 0; i < 3; i++)
    {
        int ret = syscfg_write(CFG_SLP_PARA_INFO, __this_module, __this_module_size);
        if(ret == __this_module_size)
            break;
    }

    return;
}

void SlpDataVmReset(void)
{
    memcpy(__this_module, &init, __this_module_size);
    __this_module->vm_mask = VM_MASK;
    SlpDataVmWrite();

    return;
}
