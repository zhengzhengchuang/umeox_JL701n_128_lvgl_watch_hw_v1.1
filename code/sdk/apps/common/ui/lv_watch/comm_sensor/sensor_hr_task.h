#ifndef __SENSOR_HR_TASK_H__
#define __SENSOR_HR_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "../include/ui_menu.h"

#define Ppg_Task_Name "ppg_task"

#define HrGs_Fifo_WM (Qmi8658_Fifo_WM)

enum
{
    PpgWorkHr,  //心率
    PpgWorkBo,  //血氧

    PpgWorkNone, //无工作
};
typedef u8 PpgWorkType_t;

enum
{
    PpgModeAuto,   //自动
    PpgModeManual, //手动
};
typedef u8 PpgWorkMode_t;

enum
{
    PpgMsgEnable,
    PpgMsgDisable,

    PpgMsgAgcTimeout,
    PpgMsgPpgTimeout,
};

void PpgTaskCreate(void);
int PostPpgTaskMsg(int *msg, u8 len);
void PpgTaskMsgHandle(int *msg, u8 len);

void DisablePpgModule(void);
void EnablePpgModule(u8 type, u8 mode);

/* PPG工作模式：手动、自动 */
u8 GetPpgMode(void);
void SetPpgMode(u8 mode);

/* PPG工作类型：心率、血氧 */
u8 GetPpgWorkType(void);
void SetPpgWorkType(u8 work);

/* PPG当前使能标志 */
bool GetPpgEnableFlag(void);
void SetPpgEnableFlag(bool en);

/* PPG未佩戴计数 */
u8 GetPpgUnwearCnt(void);
void SetPpgUnwearCnt(u8 cnt);

/* PPG佩戴状态 */
bool GetPpgWearStatus(void);
void SetPpgWearStatus(bool wear);

void HrGsDataFifoWrite(u8 *w_buf, u32 w_len);
u32 HrGsDataFifoRead(s16 *acc_x, s16 *acc_y, s16 *acc_z);
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif 
