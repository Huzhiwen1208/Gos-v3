#include "mod.h"

/// @brief 将 src 指向的内存拷贝到 dest 指向的内存，拷贝 size 个字节
/// @param dest 
/// @param src 
/// @param size 
void MemoryCopy(void *dest, const void *src, Size size) {
    u8 *dest8 = (u8 *)dest;
    u8 *src8 = (u8 *)src;
    while (size--) {
        *dest8++ = *src8++;
    }
}

/// @brief 释放 ptr 指向的内存，将从 ptr 开始的 size 个字节置为 0
/// @param ptr 
/// @param size 
void MemoryFree(void *ptr, Size size) {
    u8 *ptr8 = (u8 *)ptr;
    while (size--) {
        *ptr8++ = 0;
    }
}

/// @brief 将从 ptr 开始的 size 个字节均置为 value
/// @param ptr 
/// @param value 
/// @param size 
void MemorySet(void *ptr, u8 value, Size size) {
    u8 *ptr8 = (u8 *)ptr;
    while (size--) {
        *ptr8++ = value;
    }
}

/// @brief 计算字符串的长度
/// @param str 
/// @return 
Size StringLength(const char *str) {
    Size len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

/// @brief 判断两个字符串是否相等
/// @param str1 
/// @param str2 
/// @return 
Boolean StringEqual(const char *str1, const char *str2) {
    Size len1 = StringLength(str1);
    Size len2 = StringLength(str2);
    if (len1 != len2) {
        return FALSE;
    }

    for (Size i = 0; i < len1; i++) {
        if (str1[i] != str2[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

/// @brief 判断字符串 str1 是否以字符串 start 开头
/// @param str1 
/// @param start 
/// @return 
Boolean StringStartWith(const String str1, const String start) {
    Size len1 = StringLength(str1);
    Size len2 = StringLength(start);
    if (len1 < len2) {
        return FALSE;
    }

    for (Size i = 0; i < len2; i++) {
        if (str1[i] != start[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

/// @brief 将字符串转换为整数
/// @param str 
/// @return 
u32 StringToInt32(const char *str) {
    u32 result = 0;
    Size len = StringLength(str);
    for (Size i = 0; i < len; i++) {
        // if (!IsDigit(str[i])) {
        //     Panic("StringToInt32: string {%s} is not a number.", str);
        // }
        result = result * 10 + str[i] - '0';
    }
    return result;
}

/// @brief 计算字符串中所有字符的 ASCII 码之和
/// @param str 
/// @return 
u32 StringASCIITotal(const char *str) {
    u32 result = 0;
    Size len = StringLength(str);
    for (Size i = 0; i < len; i++) {
        result += str[i];
    }
    return result;
}

/// @brief 查找字符串中字符 ch 的出现次数
/// @param str 
/// @param ch 
/// @return 
u32 CharCount(const String str, char ch) {
    u32 result = 0;
    Size len = StringLength(str);
    for (Size i = 0; i < len; i++) {
        if (str[i] == ch) {
            result++;
        }
    }
    return result;
}

/// @brief 判断字符 c 是否是数字
/// @param c 
/// @return 
Boolean IsDigit(char c) {
    return c >= '0' && c <= '9';
}

/// @brief 判断字符 c 是否是字母
/// @param c 
/// @return 
Boolean IsAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/// @brief 判断字符 c 是否是大写字母
/// @param c 
/// @return  
Boolean IsUpper(char c) {
    return c >= 'A' && c <= 'Z';
}

/// @brief 判断字符 c 是否是小写字母
/// @param c 
/// @return 
Boolean IsLower(char c) {
    return c >= 'a' && c <= 'z';
}