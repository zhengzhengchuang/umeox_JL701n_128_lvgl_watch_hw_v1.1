#ifndef __VERSION_H__
#define __VERSION_H__

#ifdef __cplusplus
extern "C" {
#endif

#define Num_3_Str(a,b,c) "V" #a "." #b "." #c
#define _Version_Release_Str(a,b,c) Num_3_Str(a,b,c)

/*********************************************************************************
                                    固件版本号(Vx.x.x)                             
*********************************************************************************/
#define Fw_Version_Maintenance    1//维护版本
#define Fw_Version_Minor          0//ota版本
#define Fw_Version_Major          0//大需求变化版本
#define Fw_Version_Release _Version_Release_Str(Fw_Version_Major, \
    Fw_Version_Minor, Fw_Version_Maintenance)
    
#ifdef __cplusplus
}
#endif

#endif
