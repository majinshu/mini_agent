/*
 * markdown.h - 简单 Markdown 渲染模块
 *
 * 在 ncurses 窗口中渲染有限的 Markdown 子集。
 * 不要求完整的 Markdown 语法树解析，采用逐字符扫描+状态切换方式。
 *
 * 支持的语法：
 * - # 或 ## 开头的行 → 绿色显示整行（标题）
 * - **text**         → 红色显示 text（粗体）
 * - __text__         → 红色显示 text（粗体）
 * - `code`           → 蓝色显示 code（行内代码）
 */

#ifndef MARKDOWN_H
#define MARKDOWN_H

#include <ncurses.h>

/* 在窗口中单行渲染（xy坐标指定位置，max_w限制宽度） */
void markdown_render_line(WINDOW *win, int y, int x, const char *text, int max_w);

/* 在窗口中从start_row开始渲染多行文本，返回实际渲染的行数 */
int markdown_render(WINDOW *win, int start_row, const char *text);

/* 设置渲染时使用的颜色对编号（默认1/2/3） */
void markdown_set_colors(int green_pair, int red_pair, int blue_pair);

#endif
