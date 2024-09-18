﻿#ifndef __GOMORE_FUNC_H__
#define __GOMORE_FUNC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "workout.h"
#include "GoMoreLib.h"
#include "exampleUtils.h"
#include "GoMoreLibStruct.h"
#include "../include/ui_menu.h"

#define GoData_Ble (0)

#define GoGs_Fifo_WM (Qmi8658_Fifo_WM)

#if GoData_Ble
u8 GetGoBleFlag(void);
void SetGoBleFlag(u8 flag);
#endif

void GoMoreAlgoInit(void);
void GoMoreAlgoProcess(struct sys_time *ptime);
u8 GoMoreAlgoFactory(void);

void GoGsDataFifoWrite(u8 *w_buf, u32 w_len);
u8 GoGsDataFifoRead(float *acc_x, float *acc_y, float *acc_z, u16 max_len);
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif 
