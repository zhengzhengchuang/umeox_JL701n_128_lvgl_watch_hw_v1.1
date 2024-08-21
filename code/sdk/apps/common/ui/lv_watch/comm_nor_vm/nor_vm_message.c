#include "nor_vm_main.h"

vm_message_ctx_t w_message;
vm_message_ctx_t r_message;
static const nor_vm_type_t nor_vm_type = \
    nor_vm_type_message;

/*********************************************************************************
                              清除                                         
*********************************************************************************/
void VmMessageCtxClear(void)
{ 
    void *nor_vm_file = nor_flash_vm_file(nor_vm_type);
    if(!nor_vm_file) return;

    u8 num = VmMessageItemNum();

    while(num)
    {
        flash_common_delete_by_index(nor_vm_file, 0);
        num--;
    }
    
    return;
}

/*********************************************************************************
                              通过index删除指定信息                                         
*********************************************************************************/
void VmMessageDelByIdx(u8 index)
{
    void *nor_vm_file = nor_flash_vm_file(nor_vm_type);
    if(!nor_vm_file) return;

    flash_common_delete_by_index(nor_vm_file, index);

    return;
}

/*********************************************************************************
                              存储数量                                         
*********************************************************************************/
u8 VmMessageItemNum(void)
{
    u8 num = 0;

    void *nor_vm_file = nor_flash_vm_file(nor_vm_type);
    if(!nor_vm_file) return num;

    num = flash_common_get_total(nor_vm_file);

    if(num > Msg_Max_Num)
        num = Msg_Max_Num;

    return num;
}

/*********************************************************************************
                              获取内容                                        
*********************************************************************************/
bool VmMessageCtxByIdx(u8 idx)
{
    u8 num = VmMessageItemNum();
    if(idx >= num) return false;

    void *nor_vm_file = nor_flash_vm_file(nor_vm_type);
    int ctx_len = sizeof(vm_message_ctx_t);
    
    if(!nor_vm_file) return false;

    idx = (num - 1) - idx;
    
    memset(&r_message, 0, ctx_len);
    flash_common_read_by_index(nor_vm_file, idx, 0, \
        ctx_len, (u8 *)&r_message);

    if(r_message.check_code != Nor_Vm_Check_Code)
        return false;

    return true;
}

/*********************************************************************************
                              内容存储                                   
*********************************************************************************/
void VmMessageCtxFlashSave(void *p)
{
    bool BondFlag = GetDevBondFlag();
    if(BondFlag == false) return;

    if(!p) return;

    void *nor_vm_file = nor_flash_vm_file(nor_vm_type);
    int ctx_len = sizeof(vm_message_ctx_t);
    if(!nor_vm_file) return;

    u8 num = VmMessageItemNum();
    
    if(num >= Msg_Max_Num)
        flash_common_delete_by_index(nor_vm_file, 0);

    flash_common_write_file(nor_vm_file, 0, ctx_len, (u8 *)p);

    return;
}
