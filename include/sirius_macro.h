/********************************************************************************************
 * @name    sirius_macro.h
 * 
 * @author  胡益华
 * 
 * @date    2024-07-30
 * 
 * @brief   常规宏
********************************************************************************************/

#ifndef __SIRIUS_MACRO_H__
#define __SIRIUS_MACRO_H__

/********************************************************************************************
 * @brief   不含路径前缀的文件名宏
********************************************************************************************/
#ifndef SIRIUS_FILE
#define SIRIUS_FILE     (basename(__FILE__))
#endif  // SIRIUS_FILE

#ifndef container_of
/********************************************************************************************
 * @brief   获取结构体成员的地址偏移量
 * 
 * @param   TYPE: 结构体类型
 * @param   MEMBER: 结构体成员变量名
********************************************************************************************/
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif  // offsetof

/********************************************************************************************
 * @brief   通过结构体成员获取结构体首地址
 * 
 * @param   ptr: 结构体成员的地址
 * @param   type: 结构体类型
 * @param   member: 结构体成员变量名
********************************************************************************************/
#define container_of(ptr, type, member) ({ \
        const typeof( ((type *)0)->member ) *__mptr = (ptr); \
        (type *)( (char *)__mptr - offsetof(type, member) );})
#endif  // container_of

/********************************************************************************************
 * @brief   结构体指针变量对齐到8字节
 * 
 * @param   N: 指针变量名称
 * 
 * @note    仅支持gnuc编译
********************************************************************************************/
#ifndef SIRIUS_POINTER_ALIGN8
#if defined(__GNUC__)
#define SIRIUS_POINTER_ALIGN8(N)    unsigned char unused##N[8 - sizeof(void*)];
        // __GNUC__
#else
#warning "The macro definition [SIRIUS_POINTER_ALIGN8] only supported in the GNUC compiler"

#define SIRIUS_POINTER_ALIGN8(N)
#endif
#endif  // SIRIUS_POINTER_ALIGN8

#if defined(__GNUC__)
/********************************************************************************************
 * @brief   选择分支发生概率高
********************************************************************************************/
#ifndef  likely
#define  likely(x)      __builtin_expect(!!(x), 1)
#endif  //  likely

/********************************************************************************************
 * @brief   选择分支发生概率低
********************************************************************************************/
#ifndef unlikely
#define unlikely(x)     __builtin_expect(!!(x), 0)
#endif  // unlikely

#else
#warning "The macro definition [likely|unlikely] only supported in the GNUC compiler"

#ifndef  likely
#define  likely(x)      !!(x)
#endif  //  likely

#ifndef unlikely
#define unlikely(x)     !!(x)
#endif  // unlikely
#endif  // __GNUC__

#endif  // __SIRIUS_MACRO_H__
