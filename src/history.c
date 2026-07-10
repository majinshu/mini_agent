#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 确保 log 目录存在，没有就建个文件占位 */
static void ensure() {
    FILE *f = fopen(HISTORY_PATH, "a");
    if (f) fclose(f);
}

/* JSON 字符串转义，处理引号、换行、控制字符 */
static char *escape(const char *s) {
    if (!s) return strdup("");
    int l = strlen(s), cap = l*6+1;
    char *o = malloc(cap);
    if (!o) return NULL;
    int j = 0;
    for (int i = 0; s[i]; i++) {
        unsigned char c = s[i];
        switch (c) {
            case '"': o[j++]='\\'; o[j++]='"'; break;
            case '\\': o[j++]='\\'; o[j++]='\\'; break;
            case '\n': o[j++]='\\'; o[j++]='n'; break;
            case '\r': o[j++]='\\'; o[j++]='r'; break;
            case '\t': o[j++]='\\'; o[j++]='t'; break;
            default:
                if (c < 0x20) {
                    char b[8];
                    snprintf(b,8,"\\u%04x",c);
                    for(int k=0;b[k];) o[j++]=b[k++];
                } else o[j++]=c;
                break;
        }
    }
    o[j]='\0';
    return o;
}

/* 记录用户消息到 JSONL */
void history_log_user(const char *r) {
    ensure();
    FILE *f=fopen(HISTORY_PATH,"a");
    if(!f)return;
    char *e=escape(r);
    fprintf(f,"{\"role\":\"user\",\"message\":\"%s\"}\n",e?e:"");
    free(e); fclose(f);
}

void history_log_tool_call(const char *tn, const char *pj) {
    FILE *f=fopen(HISTORY_PATH,"a");
    if(!f)return;
    char *e=escape(pj);
    fprintf(f,"{\"role\":\"assistant\",\"message\":\"Tool_Use %s\",\"tool_call\":{\"tool\":\"%s\",\"params\":%s}}\n",
            tn, tn, e?e:"{}");
    free(e); fclose(f);
}

void history_log_tool_result(const char *tn, int ok, const char *rm, const char *rj) {
    FILE *f=fopen(HISTORY_PATH,"a");
    if(!f)return;
    char *em=escape(rm),*ej=escape(rj);
    fprintf(f,"{\"role\":\"tool\",\"message\":\"%s\",\"tool\":\"%s\",\"result\":{\"ok\":%s,\"data\":%s}}\n",
            em?em:"", tn, ok?"true":"false", ej?ej:"{}");
    free(em); free(ej); fclose(f);
}

void history_log_fixed_reply(void) {
    FILE *f=fopen(HISTORY_PATH,"a");
    if(!f)return;
    char *e=escape("## `429` Too many Requests\n\n**服务器繁忙，请稍后再试。**");
    fprintf(f,"{\"role\":\"assistant\",\"message\":\"%s\"}\n",e?e:"");
    free(e); fclose(f);
}

/* 从 JSONL 恢复历史消息到内存 */
void history_load(App *a) {
    FILE *f=fopen(HISTORY_PATH,"r");
    if(!f)return;
    char l[8192];
    while(fgets(l,sizeof(l),f)){
        int ln=strlen(l);
        while(ln>0&&(l[ln-1]=='\n'||l[ln-1]=='\r')) l[--ln]=0;
        if(!ln)continue;
        char *rs=strstr(l,"\"role\":\"");
        if(!rs)continue; rs+=8;
        char *re=strchr(rs,'"');
        if(!re)continue; *re=0;
        Role rl;
        if(!strcmp(rs,"user")) rl=ROLE_USER;
        else if(!strcmp(rs,"assistant")) rl=ROLE_ASSISTANT;
        else if(!strcmp(rs,"tool")) rl=ROLE_TOOL;
        else continue;
        char *ms=strstr(l,"\"message\":\"");
        if(!ms)continue; ms+=10;
        char *me=ms;
        while(*me&&!(*me=='"'&&*(me-1)!='\\')) me++;
        *me=0;
        app_add_message(a,rl,ms);
    }
    fclose(f);
}
