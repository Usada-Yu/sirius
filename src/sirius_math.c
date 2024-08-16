/********************************************************************************************
 * @name    sirius_math.c
 * 
 * @author  胡益华
 * 
 * @date    2024-03-29
 * 
 * @brief   常用的基础计算
********************************************************************************************/

#include "sirius_math.h"
#include "sirius_sys.h"

/********************************************************************************************
 * @brief   返回若干个整型数字中的最大值
 * 
 * @param   args_num: 整型数字的总个数
 * @param   ...: 若干的整型数字
 * 
 * @note    (1) args_num为必填项
 *          (2) 举例：若传入整型数字总数为8，那么args_num填写8即可；
 *          args_num填写大于8，如10，那么会比较8个整型数字；
 *          args_num填写小于8，如6，那么会比较前6个整型数字
 *          (3) 函数基于可变参数列表编写。
 *          va_arg宏读取参数时会根据传入参数类型大小的偏移读取，故传参时必须传递整型类型
********************************************************************************************/
inline int sirius_math_max_int(unsigned int args_num, ...)
{
    va_list args;
    va_start(args, args_num);

    int max_val = INT_MIN;
    for (unsigned int i = 0; i < args_num; i++) {
        int arg = va_arg(args, int);
        max_val = arg > max_val ? arg : max_val;
    }

    va_end(args);
    return max_val;
}

/********************************************************************************************
 * @brief   返回若干个整型数字中的最小值
 * 
 * @param   args_num: 整型数字的总个数
 * @param   ...: 若干的整型数字
 * 
 * @note    (1) args_num为必填项
 *          (2) 举例：若传入整型数字总数为8，那么args_num填写8即可；
 *          args_num填写大于8，如10，那么会比较8个整型数字；
 *          args_num填写小于8，如6，那么会比较前6个整型数字
 *          (3) 函数基于可变参数列表编写。
 *          va_arg宏读取参数时会根据传入参数类型大小的偏移读取，故传参时必须传递整型
********************************************************************************************/
inline int sirius_math_min_int(unsigned int args_num, ...)
{
    va_list args;
    va_start(args, args_num);

    int min_val = INT_MAX;
    for (unsigned int i = 0; i < args_num; i++) {
        int arg = va_arg(args, int);
        min_val = arg < min_val ? arg : min_val;
    }

    va_end(args);
    return min_val;
}

/********************************************************************************************
 * @brief   返回若干个双精度浮点型数字中的最大值
 * 
 * @param   args_num: 双精度浮点型数字的总个数
 * @param   ...: 若干的双精度浮点型数字
 * 
 * @note    (1) args_num为必填项；
 *          (2) 举例：若传入双精度浮点型数字总数为8，那么args_num填写8即可；
 *          args_num填写大于8，如10，那么会比较8个双精度浮点型数字；
 *          args_num填写小于8，如6，那么会比较前6个双精度浮点型数字
 *          (3) 函数基于可变参数列表编写。
 *          对于浮点型，va_arg宏只能以double类型读取而不能以float类型读取；
 *          va_arg宏读取参数时会根据传入参数类型大小的偏移读取，故传参时必须传递浮点型
********************************************************************************************/
inline double sirius_math_max_dbl(unsigned int args_num, ...)
{
    va_list args;
    va_start(args, args_num);

    double max_val = DBL_MIN;
    for (unsigned int i = 0; i < args_num; i++) {
        double arg = va_arg(args, double);
        max_val = arg > max_val ? arg : max_val;
    }

    va_end(args);
    return max_val;
}

/********************************************************************************************
 * @brief   返回若干个双精度浮点型数字中的最小值
 * 
 * @param   args_num: 双精度浮点型数字的总个数
 * @param   ...: 若干的双精度浮点型数字
 * 
 * @note    (1) args_num为必填项；
 *          (2) 举例：若传入双精度浮点型数字总数为8，那么args_num填写8即可；
 *          args_num填写大于8，如10，那么会比较8个双精度浮点型数字；
 *          args_num填写小于8，如6，那么会比较前6个双精度浮点型数字
 *          (3) 函数基于可变参数列表编写。
 *          对于浮点型，va_arg宏只能以double类型读取而不能以float类型读取；
 *          va_arg宏读取参数时会根据传入参数类型大小的偏移读取，故传参时必须传递浮点型
********************************************************************************************/
inline double sirius_math_min_dbl(unsigned int args_num, ...)
{
    va_list args;
    va_start(args, args_num);

    double min_val = DBL_MAX;
    for (unsigned int i = 0; i < args_num; i++) {
        double arg = va_arg(args, double);
        min_val = arg < min_val ? arg : min_val;
    }

    va_end(args);
    return min_val;
}
