/********************************************************************************************
 * @name    sirius_errno.h
 * 
 * @author  胡益华
 * 
 * @date    2024-07-30
 * 
 * @brief   错误码
********************************************************************************************/

#ifndef __SIRIUS_ERRNO_H__
#define __SIRIUS_ERRNO_H__

#define SIRIUS_OK                        (0)            // 通用正确
#define SIRIUS_ERR                       (-1)           // 通用错误

#define SIRIUS_ERR_TIMEOUT               (-10000)       // 超时
#define SIRIUS_ERR_NULL_POINTER          (-10001)       // 指针为空
#define SIRIUS_ERR_MEMORY_ALLOC          (-10002)       // 内存申请失败
#define SIRIUS_ERR_CACHE_OVERFLOW        (-10003)       // 缓存不足
#define SIRIUS_ERR_INVALID_ENTRY         (-10004)       // 函数入参无效
#define SIRIUS_ERR_INVALID_PARAMETER     (-10005)       // 参数无效
#define SIRIUS_ERR_INIT_REPEATED         (-10006)       // 重复初始化
#define SIRIUS_ERR_NOT_INIT              (-10007)       // 未初始化

#endif  // __SIRIUS_ERRNO_H__
