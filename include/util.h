/*
 * util.h - 通用工具函数模块
 *
 * 提供字符串处理、路径处理和JSON转义等辅助功能。
 * 这些函数在各模块中都会用到，独立成一个单元。
 */

#ifndef UTIL_H
#define UTIL_H

/* 安全字符串复制：确保目标字符串以 \0 结尾，不溢出 */
void safe_strcpy(char *dst, const char *src, int max_len);

/**
 * JSON字符串转义：将普通字符串转为合法的JSON字符串字面量
 * 处理：双引号\"、反斜杠\\、换行\n、回车\r、制表符\t、控制字符
 * 返回值需要调用者 free()
 */
char *json_escape(const char *s);

/**
 * 路径显示转换：将完整路径中的家目录替换为 ~
 * 例如 /home/xiaoruan/mini-agent → ~/mini-agent
 */
void path_to_display(const char *cwd, const char *home, char *out, int out_size);

#endif
