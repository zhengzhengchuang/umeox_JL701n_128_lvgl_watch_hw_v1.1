#include "../lv_watch.h"

extern void swapX(const uint8_t *src, uint8_t *dst, int len);

static IndexIO mInput;

static void *sdkMem = NULL;
static char *prevData = NULL;
static bool algo_init_flag = false;

#define ACC_MAX 30
static float accX[ACC_MAX];
static float accY[ACC_MAX];
static float accZ[ACC_MAX];

#define FifoSize (GoGs_Fifo_WM*5*6)
static u32 Widx;
static u32 Wlen;
//static u8 FifoRflag;
static u8 WFifoData[FifoSize];

static int8_t slp_stages[2880];

static char *GoMoreDevIdParse(void)
{
    static char id_buf[13];
    memset(id_buf, 0, 13);

    static u8 mac_addr[6];
    memset(mac_addr, 0, 6);
    swapX(GetDevBleMac(), mac_addr, 6);

    u8 buf_idx = 0;
    u8 mac_idx = 0;
    for(u8 i = 0; i < 17; i++)
    {
        if(((i + 1) % 3) == 0)
		{
            mac_idx++;
			continue;
		}else if(((i + 1) % 3) == 1)
		{
			id_buf[buf_idx] = ((mac_addr[mac_idx]>>4)&0x0f) > 9 ? \
			    ((mac_addr[mac_idx]>>4)&0x0f) + 0x37:((mac_addr[mac_idx] >> 4)&0x0f) + 0x30;
		}else if(((i + 1) % 3) == 2)
		{
			id_buf[buf_idx] = (mac_addr[mac_idx]&0x0f) > 9 ? \
			    ((mac_addr[mac_idx])&0x0f) + 0x37:(mac_addr[mac_idx]&0x0f) + 0x30;
		}

        buf_idx++;
    }

    return id_buf;
}

int8_t gomore_pkey_get(char *pKey, uint8_t Size)
{
    if(pKeyExample == NULL)
        return -2;

    Size = strlen(pKeyExample);

    memcpy(pKey, pKeyExample, Size);

    return Size;
}

int8_t gomore_device_id_get(char *Device_ID, uint8_t Size)
{
    if(deviceIdExample == NULL)
        return -2;

    Size = strlen(deviceIdExample);

    memcpy(Device_ID, deviceIdExample, Size);

    return Size;
}

#if Sleep_Debug
static const float fakeSleepAcc[25] = {0.0f};
static const float fakeWakeAcc[25] = {0.0f,994.0f,1927.0f,2738.0f,3377.0f,3804.0f,3992.0f,3929.0f,3619.0f,3082.0f,2351.0f,1472.0f,501.0f,-501.0f,-1472.0f,-2351.0f,-3082.0f,-3619.0f,-3929.0f,-3992.0f,-3804.0f,-3377.0f,-2738.0f,-1927.0f,-994.0f,};

static const float fakeSleepPpg[25] = {0.0f,994.0f,1927.0f,2738.0f,3377.0f,3804.0f,3992.0f,3929.0f,3619.0f,3082.0f,2351.0f,1472.0f,501.0f,-501.0f,-1472.0f,-2351.0f,-3082.0f,-3619.0f,-3929.0f,-3992.0f,-3804.0f,-3377.0f,-2738.0f,-1927.0f,-994.0f,};
static const float fakeWakePpg[25] = {0.0f,1963.615014f,3421.457040f,3998.026241f,3544.814316f,2178.556140f,251.162078f,-1740.924397f,-3284.596836f,-3982.247858f,-3654.181830f,-2384.899499f,-501.332934f,1511.363147f,3134.773829f,3950.753362f,3749.127957f,2581.830750f,749.525258f,-1275.837237f,-2972.579302f,-3903.667047f,-3829.277990f,-2768.572695f,-994.759548f,};
#endif

#if GoData_Ble
static u8 go_ble_flag;
u8 GetGoBleFlag(void)
{
    return go_ble_flag;
}
void SetGoBleFlag(u8 flag)
{
    go_ble_flag = flag;
    return;
}
#endif

void GoMoreAlgoProcess(struct sys_time *ptime)
{
    if(algo_init_flag == false) return;
    //printf("____________algo_init_flag = %d\n", algo_init_flag);


#if !Sleep_Debug
    if(ptime->year <= Sys_Def_Year) return;

    mInput.timestamp = UtcTimeToSec(ptime);
    int time_zone = GetVmParaCacheByLabel(vm_label_time_zone);
    mInput.timeZoneOffset = (int16_t)((time_zone/10.0f)*60);

    /* acc数据 */
    u8 acc_len = GoGsDataFifoRead(accX, accY, accZ, ACC_MAX);
    mInput.accX = accX;
    mInput.accY = accY;
    mInput.accZ = accZ;
    mInput.accLength = acc_len;
    //printf("acc_len = %d\n", acc_len);
    // for(u16 i = 0; i < acc_len; i++)
    //     printf("%d:x = %f  y = %f  z = %f\n", i, accX[i], accY[i], accZ[i]);

    /* ppg数据 */
    mInput.ppg1 = GetSlpHrRawData();
    mInput.ppgLength = GetSlpHrRawDataLen();
    mInput.ppgNumChannels = 1;
    // for(u8 i = 0; i < mInput.ppgLength; i++)
    // {
    //     printf("[%d]:ppg = %f\n", i, mInput.ppg1[i]);
    // }

#if 1
    bool fall_asleep = GetFallAsleepFlag();
    if(fall_asleep == true)
    {
        /* 进入入睡 */
        u8 slp_ppg_en = GetSlpPpgEnState();
        if(slp_ppg_en == 1)
        {
            /* 算法开启ppg讯号，穿戴状态由ppg传感器决定 */
            mInput.wristOff = GetSlpPpgWearState();
            //printf("wristOff = %d\n", mInput.wristOff);
        }
    }
#endif
    // for(u8 i = 0; i < mInput.ppgLength; i++)
    //     printf("ppg1[%d] = %f\n", i, mInput.ppg1[i]);

    int16_t flag = updateIndex(&mInput);
    if(flag < 0) return;

    /* pedo data */
    SedSetSteps(mInput.stepCountOut);
    PedoDataHandle(mInput.stepCountOut, mInput.activityKcalOut, DistanceCalc(mInput.stepCountOut));
    SetPedoDataVmCtxCache(mInput.stepCountOut, mInput.activityKcalOut, DistanceCalc(mInput.stepCountOut));

    /* sleep相关 */
    SleepStatusOutHandle(ptime, mInput.sleepPeriodStatusOut);
    SleepPpgSensorOnOff(ptime, mInput.sleepStagePpgOnOffOut);
#else
    bool slp_en = GetSleepEnState();
    if(slp_en == false) return;

     mInput.timestamp = UtcTimeToSec(ptime);
    int time_zone = GetVmParaCacheByLabel(vm_label_time_zone);
    mInput.timeZoneOffset = (int16_t)((time_zone/10.0f)*60);

    static u32 cnt = 0;
    // printf("____%s:cnt = %d\n", __func__, cnt);
    // printf("____%s:timestamp = %d\n", __func__, mInput.timestamp);
    u32 slp_6_hour = 6*60*60;
    if(cnt < slp_6_hour)
    {
        mInput.accX = fakeSleepAcc;
        mInput.accY = fakeSleepAcc;
        mInput.accZ = fakeSleepAcc;
        mInput.accLength = 25;

        mInput.ppg1 = fakeSleepPpg;
        mInput.ppgLength = 25;
        mInput.ppgNumChannels = 1;
    }else
    {
        mInput.accX = fakeWakeAcc;
        mInput.accY = fakeWakeAcc;
        mInput.accZ = fakeWakeAcc;
        mInput.accLength = 25;

        mInput.ppg1 = fakeWakePpg;
        mInput.ppgLength = 25;
        mInput.ppgNumChannels = 1;
    }
#if 0
    if(cnt < 60*5*60)//入睡5个小时
    {
        mInput.accX = fakeSleepAcc;
        mInput.accY = fakeSleepAcc;
        mInput.accZ = fakeSleepAcc;
        mInput.accLength = 25;

        mInput.ppg1 = fakeSleepPpg;
        mInput.ppgLength = 25;
        mInput.ppgNumChannels = 1;
            // set spo2 value if osa output is required.
            // mInput.spo2 = ...;
    }else if(cnt >= 60*5*60 && cnt < 60*5*60 + 20*60)//清醒20分钟
    {
        mInput.accX = fakeWakeAcc;
        mInput.accY = fakeWakeAcc;
        mInput.accZ = fakeWakeAcc;
        mInput.accLength = 25;

        mInput.ppg1 = fakeWakePpg;
        mInput.ppgLength = 25;
        mInput.ppgNumChannels = 1;
    }else if(cnt >= 60*5*60 + 20*60 && cnt < 2*60*5*60 + 20*60)//又入睡5小时
    {
        mInput.accX = fakeSleepAcc;
        mInput.accY = fakeSleepAcc;
        mInput.accZ = fakeSleepAcc;
        mInput.accLength = 25;

        mInput.ppg1 = fakeSleepPpg;
        mInput.ppgLength = 25;
        mInput.ppgNumChannels = 1;
    }else//后面清醒
    {
        mInput.accX = fakeWakeAcc;
        mInput.accY = fakeWakeAcc;
        mInput.accZ = fakeWakeAcc;
        mInput.accLength = 25;

        mInput.ppg1 = fakeWakePpg;
        mInput.ppgLength = 25;
        mInput.ppgNumChannels = 1;
    }
#endif
    cnt++;

    int16_t flag = updateIndex(&mInput);
    if(flag < 0) return;

#if 0
    if (mInput.sleepPeriodStatusOut & 0x02) {
            // Detected Sleep Period On Event
            printf("______________sleep On Event\n");

    } else if (mInput.sleepPeriodStatusOut & 0x04){
        // Detected Sleep Period Off Event
        printf("______________sleep Off Event\n");
        if (mInput.sleepPeriodStatusOut & 0x08) {
            // The Sleep Period is Valid, Get Sleep Summary
            printf("The Sleep Period is Valid, Get Sleep Summary\n");
            getSummary(&slp_in, &slp_out);

            // if (mInput.osaStatusOut == 1) {
            //     // Get Osa Result
            //     printf("OSA Result=%d, Spo2 Conf=%d, Osa status=%d\n", mInput.osaOut, mInput.osaSpo2ConfOut, mInput.osaStatusOut);
            // }
        }
    }
#endif

    //printf("sleepStageOut = %d\n", mInput.sleepStageOut);
    SleepStatusOutHandle(ptime, mInput.sleepPeriodStatusOut);
#endif

#if GoData_Ble
    // if(GetGoBleFlag() == 0)
    //     return;

    SleepConfig slp_cfg;
    getSleepConfig(0, &slp_cfg);
    if(slp_cfg.disableAutoDetection == 1)
        return;

    u8 idx;
    u8 nfy_buf[130];//30*4
    memset(nfy_buf, 0x00, sizeof(nfy_buf));

    /* TIMESTAMP */
    idx = 0;
    nfy_buf[idx++] = 0x3A;
    nfy_buf[idx++] = 0x01;
    nfy_buf[idx++] = (mInput.timestamp>>24)&0xff;
    nfy_buf[idx++] = (mInput.timestamp>>16)&0xff;
    nfy_buf[idx++] = (mInput.timestamp>>8)&0xff;
    nfy_buf[idx++] = (mInput.timestamp>>0)&0xff;
    nfy_buf[idx++] = calc_crc(nfy_buf, idx);
    umeox_common_le_notify_data(nfy_buf, idx);
    //printf("timestamp = %d\n", mInput.timestamp);

    /* PPG1 */
    idx = 0;
    nfy_buf[idx++] = 0x3A;
    nfy_buf[idx++] = 0x02;
    nfy_buf[idx++] = mInput.ppgLength;
    int ppg1_data;
    for(u8 i = 0; i < mInput.ppgLength; i++)
    {
        ppg1_data = (int)(mInput.ppg1[i]);
        nfy_buf[idx++] = (ppg1_data>>24)&0xff;
        nfy_buf[idx++] = (ppg1_data>>16)&0xff;
        nfy_buf[idx++] = (ppg1_data>>8)&0xff;
        nfy_buf[idx++] = (ppg1_data>>0)&0xff;
        //printf("%d:ppg1_data = %d\n", i, ppg1_data);
    }
    nfy_buf[idx++] = calc_crc(nfy_buf, idx);
    umeox_common_le_notify_data(nfy_buf, idx);

    /* ACCX */
    idx = 0;
    nfy_buf[idx++] = 0x3A;
    nfy_buf[idx++] = 0x03;
    nfy_buf[idx++] = mInput.accLength;
    int accx_data;
    for(u8 i = 0; i < mInput.accLength; i++)
    {
        accx_data = (int)(mInput.accX[i]*(100.0f));
        nfy_buf[idx++] = (accx_data>>24)&0xff;
        nfy_buf[idx++] = (accx_data>>16)&0xff;
        nfy_buf[idx++] = (accx_data>>8)&0xff;
        nfy_buf[idx++] = (accx_data>>0)&0xff;
        //printf("%d:accx_data = %d\n", i, accx_data);
    }
    nfy_buf[idx++] = calc_crc(nfy_buf, idx);
    //put_buf(nfy_buf);
    umeox_common_le_notify_data(nfy_buf, idx);

    /* ACCY */
    idx = 0;
    nfy_buf[idx++] = 0x3A;
    nfy_buf[idx++] = 0x04;
    nfy_buf[idx++] = mInput.accLength;
    int accy_data;
    for(u8 i = 0; i < mInput.accLength; i++)
    {
        accy_data = (int)(mInput.accY[i]*(100.0f));
        nfy_buf[idx++] = (accy_data>>24)&0xff;
        nfy_buf[idx++] = (accy_data>>16)&0xff;
        nfy_buf[idx++] = (accy_data>>8)&0xff;
        nfy_buf[idx++] = (accy_data>>0)&0xff;
        //printf("%d:accy_data = %d\n", i, accy_data);
    }
    nfy_buf[idx++] = calc_crc(nfy_buf, idx);
    umeox_common_le_notify_data(nfy_buf, idx);

    /* ACCZ */
    idx = 0;
    nfy_buf[idx++] = 0x3A;
    nfy_buf[idx++] = 0x05;
    nfy_buf[idx++] = mInput.accLength;
    int accz_data;
    for(u8 i = 0; i < mInput.accLength; i++)
    {
        accz_data = (int)(mInput.accZ[i]*(100.0f));
        nfy_buf[idx++] = (accz_data>>24)&0xff;
        nfy_buf[idx++] = (accz_data>>16)&0xff;
        nfy_buf[idx++] = (accz_data>>8)&0xff;
        nfy_buf[idx++] = (accz_data>>0)&0xff;
        //printf("%d:accz_data = %d\n", i, accz_data);
    }
    nfy_buf[idx++] = calc_crc(nfy_buf, idx);
    umeox_common_le_notify_data(nfy_buf, idx);

    /* wistOff */
    idx = 0;
    nfy_buf[idx++] = 0x3A;
    nfy_buf[idx++] = 0x06;
    nfy_buf[idx++] = mInput.wristOff;
    nfy_buf[idx++] = calc_crc(nfy_buf, idx);
    umeox_common_le_notify_data(nfy_buf, idx);

    /* HR */
    idx = 0;
    nfy_buf[idx++] = 0x3A;
    nfy_buf[idx++] = 0x07;
    nfy_buf[idx++] = -1;
    nfy_buf[idx++] = calc_crc(nfy_buf, idx);
    umeox_common_le_notify_data(nfy_buf, idx);

    /* sleepPeriodStatusOut */
    idx = 0;
    nfy_buf[idx++] = 0x3A;
    nfy_buf[idx++] = 0x08;
    nfy_buf[idx++] = mInput.sleepPeriodStatusOut;
    nfy_buf[idx++] = calc_crc(nfy_buf, idx);
    umeox_common_le_notify_data(nfy_buf, idx);
    //printf("sleepPeriodStatusOut = %d\n", mInput.sleepPeriodStatusOut);

    /* sleepStagePpgOnOffOut */
    idx = 0;
    nfy_buf[idx++] = 0x3A;
    nfy_buf[idx++] = 0x09;
    nfy_buf[idx++] = mInput.sleepStagePpgOnOffOut;
    nfy_buf[idx++] = calc_crc(nfy_buf, idx);
    umeox_common_le_notify_data(nfy_buf, idx);
    //printf("sleepStagePpgOnOffOut = %d\n", mInput.sleepStagePpgOnOffOut);

    /* sleepStageOut */
    idx = 0;
    nfy_buf[idx++] = 0x3A;
    nfy_buf[idx++] = 0x0A;
    nfy_buf[idx++] = mInput.sleepStageOut;
    nfy_buf[idx++] = calc_crc(nfy_buf, idx);
    umeox_common_le_notify_data(nfy_buf, idx);
    //printf("sleepStageOut = %d\n", mInput.sleepStageOut);

    /* sleepPeriodStartTSOut */
    idx = 0;
    nfy_buf[idx++] = 0x3A;
    nfy_buf[idx++] = 0x0B;
    nfy_buf[idx++] = (mInput.sleepPeriodStartTSOut>>24)&0xff;
    nfy_buf[idx++] = (mInput.sleepPeriodStartTSOut>>16)&0xff;
    nfy_buf[idx++] = (mInput.sleepPeriodStartTSOut>>8)&0xff;
    nfy_buf[idx++] = (mInput.sleepPeriodStartTSOut>>0)&0xff;
    nfy_buf[idx++] = calc_crc(nfy_buf, idx);
    umeox_common_le_notify_data(nfy_buf, idx);
    //printf("sleepPeriodStartTSOut = %d\n", mInput.sleepPeriodStartTSOut);

    /* sleepPeriodEndTSOut */
    idx = 0;
    nfy_buf[idx++] = 0x3A;
    nfy_buf[idx++] = 0x0C;
    nfy_buf[idx++] = (mInput.sleepPeriodEndTSOut>>24)&0xff;
    nfy_buf[idx++] = (mInput.sleepPeriodEndTSOut>>16)&0xff;
    nfy_buf[idx++] = (mInput.sleepPeriodEndTSOut>>8)&0xff;
    nfy_buf[idx++] = (mInput.sleepPeriodEndTSOut>>0)&0xff;
    nfy_buf[idx++] = calc_crc(nfy_buf, idx);
    umeox_common_le_notify_data(nfy_buf, idx);
    //printf("sleepPeriodEndTSOut = %d\n", mInput.sleepPeriodEndTSOut);
#endif

    return;
}
 
static void GoMoreAlgoUninit(void)
{
    if(sdkMem)
    {
        stopHealthSession(sdkMem);
        free(sdkMem);
        sdkMem = NULL;
    } 

    if(prevData)
    {
        free(prevData);
        prevData = NULL;
    }
    algo_init_flag = false;
    //printf("_____GoMoreAlgoUninit\n");
    return;
}

void GoMoreAlgoInit(void)
{
    /* 设备无绑定，不初始化算法 */
    bool BondFlag = GetDevBondFlag();
    if(BondFlag == false) return;

    if(algo_init_flag == true)   
        GoMoreAlgoUninit();
     
    /* 解析设备id */
    //deviceIdExample = GoMoreDevIdParse();
    printf("deviceIdExample = %s\n", deviceIdExample);

#if 1
    struct sys_time utc_time;
    GetUtcTime(&utc_time);
    int rtc_current_time = (int)UtcTimeToSec(&utc_time);
    printf("rtc_current_time = %d\n", rtc_current_time);
#else
    int rtc_current_time = 1688169610;
#endif

    //Sdk Auth
    workoutAuthParameters auth;
    setWorkoutAuthParametersExample(rtc_current_time, &auth);
    int16_t ret = setAuthParameters(&auth);
    if(ret < 0) return;

    // Allocate Memory 
    u32 MemSize = getMemSizeHealthFrame();
    u32 DataSize = getPreviousDataSize();
    sdkMem = malloc(MemSize);
    prevData = (char*)malloc(DataSize);

    memset(prevData, 0, DataSize);

    /* 算法用户信息初始化 */
    float userInfo[7];
    userInfo[0] = User_Info.age*(1.0f);
    userInfo[1] = User_Info.gender*(1.0f);
    userInfo[2] = User_Info.height*(1.0f);
    userInfo[3] = User_Info.weight*(1.0f);
    userInfo[4] = (220 - User_Info.age)*(1.0f);
    userInfo[5] = 70.0f;
    userInfo[6] = 40.0f;
    int r = healthIndexInitUser(sdkMem, rtc_current_time, userInfo, prevData);
    if(r < 0) return;
    for(u8 i = 0; i < 7; i++)
        printf("userInfo[%d] = %f\n", i, userInfo[i]);

    /* 上电先关闭自动检测，时间范围：20:00~8:00 */
    SleepConfig slp_cfg;
    getSleepConfig(0, &slp_cfg);
    slp_cfg.disableAutoDetection = 1;
    int16_t cfg = setSleepConfig(&slp_cfg);
    if(cfg == 0) printf("setSleepConfig success\n");

    memset(&slp_in, 0, sizeof(struct SleepSummaryInput));
    memset(&slp_out, 0, sizeof(struct SleepSummaryOutput));
    slp_out.stages = slp_stages;
    // printf("minNapLength = %d\n", slp_cfg.minNapLength);
    // printf("maxGapLength = %d\n", slp_cfg.maxGapLength);
    // printf("disablePeriodFinetuning = %d\n", slp_cfg.disablePeriodFinetuning);
    // printf("disableAutoDetection = %d\n", slp_cfg.disableAutoDetection);
    // printf("longSleepPeriod[0] = %d\n", slp_cfg.longSleepPeriod[0]);
    // printf("longSleepPeriod[1] = %d\n", slp_cfg.longSleepPeriod[1]);

    setIndexIODefaultValue(&mInput);

    mInput.timestamp = rtc_current_time;
#if 0
    // mInput.timestamp = rtc_current_time;
    // int time_zone = GetVmParaCacheByLabel(vm_label_time_zone);
    // mInput.timeZoneOffset = (int16_t)((time_zone/10.0f)*60);

    mInput.timestamp = rtc_current_time;
    mInput.timeZoneOffset = 480; // utc +8

    int sleep_minutes = 60;
    int akake_minutes = 30;
    printf("_______sleep_minutes = %d, akake_minutes = %d\n", sleep_minutes, akake_minutes);
    for(int i = 0; i < (sleep_minutes + akake_minutes)*60; i++) 
	{
        mInput.timestamp++;

        if (i < sleep_minutes * 60) {
            mInput.accX = fakeSleepAcc;
            mInput.accY = fakeSleepAcc;
            mInput.accZ = fakeSleepAcc;
            mInput.accLength = 25;

            mInput.ppg1 = fakeSleepPpg;
            mInput.ppgLength = 25;
            mInput.ppgNumChannels = 1;
            // set spo2 value if osa output is required.
            // mInput.spo2 = ...;
        } else {
            mInput.accX = fakeWakeAcc;
            mInput.accY = fakeWakeAcc;
            mInput.accZ = fakeWakeAcc;
            mInput.accLength = 25;

            mInput.ppg1 = fakeWakePpg;
            mInput.ppgLength = 25;
            mInput.ppgNumChannels = 1;
            printf("________fakeWakePpg\n");
        }

#if 0
        updateIndex(&mInput);
        if (mInput.sleepPeriodStatusOut & 0x02) {
            // Detected Sleep Period On Event
            printf("______________sleep On Event\n");

        } else if (mInput.sleepPeriodStatusOut & 0x04){
            // Detected Sleep Period Off Event
            printf("______________sleep Off Event\n");
            if (mInput.sleepPeriodStatusOut & 0x08) {
                // The Sleep Period is Valid, Get Sleep Summary
                printf("The Sleep Period is Valid, Get Sleep Summary\n");
                getSummary(&slp_in, &slp_out);

                // if (mInput.osaStatusOut == 1) {
                //     // Get Osa Result
                //     printf("OSA Result=%d, Spo2 Conf=%d, Osa status=%d\n", mInput.osaOut, mInput.osaSpo2ConfOut, mInput.osaStatusOut);
                // }
            }
        }
#endif
    }
#endif

    algo_init_flag = true;

    printf("+++++++algo_init_flag\n");

    return;
}

u8 GoMoreAlgoFactory(void)
{
    if(algo_init_flag == true)
        GoMoreAlgoUninit();

    int rtc_current_time = 1701388800;

    //Sdk Auth
    workoutAuthParameters auth;
    setWorkoutAuthParametersExample(rtc_current_time, &auth);
    int16_t ret = setAuthParameters(&auth);
    if(ret < 0) return 0;

    // Allocate Memory 
    u32 MemSize = getMemSizeHealthFrame();
    u32 DataSize = getPreviousDataSize();
    sdkMem = malloc(MemSize);
    prevData = (char*)malloc(DataSize);

    memset(prevData, 0, DataSize);

    /* 算法用户信息初始化 */
    float userInfo[7];
    userInfo[0] = User_Info.age*(1.0f);
    userInfo[1] = User_Info.gender*(1.0f);
    userInfo[2] = User_Info.height*(1.0f);
    userInfo[3] = User_Info.weight*(1.0f);
    userInfo[4] = (220 - User_Info.age)*(1.0f);
    userInfo[5] = 70.0f;
    userInfo[6] = 40.0f;
    int r = healthIndexInitUser(sdkMem, rtc_current_time, userInfo, prevData);
    if(r < 0) return 0;

    GoMoreAlgoUninit();

    return 1;
}

void GoGsDataFifoWrite(u8 *w_buf, u32 w_len)
{
    bool BondFlag = GetDevBondFlag();
    if(BondFlag == false) return;

    /*为了读、写数据同步*/
    //if(FifoRflag == 1) return;

    u8 *pW = WFifoData;
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
    Wlen += w_len;

    return;
}

//static const u32 GoGsL = 25;
u8 GoGsDataFifoRead(float *acc_x, float *acc_y, float *acc_z, u16 max_len)
{
    //static u32 Rlen = 0;
    static u8 count = 0;
    static u8 RFifoData[ACC_MAX*6] = {0};

    u8 *pR = RFifoData;
    u8 *pW = WFifoData;

    if(Wlen == 0) goto __end;

    //FifoRflag = 1;
    memset(pR, 0, ACC_MAX*6);
    u32 Rlen = Wlen;
    Widx = 0; Wlen = 0;
    u32 mod = Rlen%6;
    Rlen -= mod;
    count = Rlen/6;
    if(count > max_len)
        count = max_len;
    memcpy(pR, pW, count*6);
    
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
        
        acc_x[i] = (float)((-acc_data[0]*1000.0f)/GetGsACCSsvt());
        acc_y[i] = (float)((-acc_data[1]*1000.0f)/GetGsACCSsvt());
        acc_z[i] = (float)((acc_data[2]*1000.0f)/GetGsACCSsvt());

        idx += 6;
    } 

    return count;
}
