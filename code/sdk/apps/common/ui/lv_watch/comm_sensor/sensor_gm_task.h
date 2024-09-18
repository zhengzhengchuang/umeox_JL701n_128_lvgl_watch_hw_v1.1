#ifndef __SENSOR_GM_TASK_H__
#define __SENSOR_GM_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "../include/ui_menu.h"
#include "../../../device/gSensor/qmi8658/qmi8658.h"
#include "../../../device/m_sensor/qmc6309/qmc6309.h"

#define Gm_Task_Name "gm_task"

#define GmGs_Fifo_WM (Qmi8658_Fifo_WM)

#define GM_DATA_DEBUG 0
#if GM_DATA_DEBUG
#define GM_LEN (200)//30
#define GM_CNT (30)
extern float gm_data0[GM_LEN];
extern float gm_data1[GM_LEN];
extern float gm_data2[GM_LEN];
#endif

#define GM_DATA_BLE (0)

enum
{
    GmMsgProcess,
    GmMsgEnable,
    GmMsgDisable,
};

typedef struct
{
    u16 vm_mask;

    bool cali_succ;
    float calipara[4];
}GmCaliInfoPara_t;
extern GmCaliInfoPara_t Cali_Info;

void GmTaskCreate(void);
int PostGmTaskMsg(int *msg, u8 len);
void GmTaskMsgHandle(int *msg, u8 len);

void EnableGmModule(void);
void DisableGmModule(void);

s16 GetGmYawAngle(void);
void SetGmYawAngle(s16 yaw);

bool GetGmEnableFlag(void);
void SetGmEnableFlag(bool f);

bool GetGmCaliSucc(void);
void SetGmCaliSucc(bool f);

void GmGsDataFifoWrite(u8 *w_buf, u32 w_len);
u32 GmGsDataFifoRead(s16 *acc_x, s16 *acc_y, s16 *acc_z);

void GmCaliInfoRead(void);
void GmCaliInfoWrite(void);
void GmCaliInfoReset(void);
void GmCaliInfoUpdate(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif 
