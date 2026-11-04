#include "input/input.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256
static char *trim(char *s) { while (isspace((unsigned char)*s)) ++s; char *e = s + strlen(s); while (e > s && isspace((unsigned char)e[-1])) *--e = '\0'; return s; }
static int equal(const char *a, const char *b) { while (*a && *b && tolower((unsigned char)*a) == tolower((unsigned char)*b)) { ++a; ++b; } return !*a && !*b; }
static InputAction command(const char *s) { if (equal(s,"q")||equal(s,"quit")||equal(s,"exit")) return INPUT_QUIT; if (equal(s,"c")||equal(s,"clear")||equal(s,"reset")) return INPUT_RESET; return INPUT_OK; }
static int get_line(char line[BUFFER_SIZE]) { if (!fgets(line, BUFFER_SIZE, stdin)) return 0; if (strchr(line,'\n') || feof(stdin)) return 1; int ch; while ((ch=getchar())!='\n' && ch!=EOF) {} return -1; }
int parse_int(const char *text, int *out) { if (!text||!out||!*text) return 0; errno=0; char *end; long v=strtol(text,&end,10); while (isspace((unsigned char)*end)) ++end; if (end==text||*end||errno==ERANGE||v<INT_MIN||v>INT_MAX) return 0; *out=(int)v; return 1; }
int parse_money_cents(const char *text, int64_t *out) {
    if (!text || !out) return 0; char buffer[BUFFER_SIZE]; if (strlen(text) >= sizeof(buffer)) return 0; snprintf(buffer,sizeof(buffer),"%s",text); char *p=trim(buffer); if (*p=='$') ++p; if (!isdigit((unsigned char)*p)) return 0;
    int64_t whole=0; while (isdigit((unsigned char)*p)) { if (whole>(INT64_MAX-9)/10) return 0; whole=whole*10+(*p++-'0'); }
    int cents=0; if (*p=='.') { ++p; if (!isdigit((unsigned char)*p)) return 0; cents=(*p++-'0')*10; if (isdigit((unsigned char)*p)) cents+=*p++-'0'; if (isdigit((unsigned char)*p)) return 0; }
    if (*trim(p)!='\0' || whole>(INT64_MAX-cents)/100) return 0; *out=whole*100+cents; return 1;
}
InputAction read_int_in_range(const char *prompt,int min,int max,int *out) { if(!out)return INPUT_ERR_NULL; char line[BUFFER_SIZE]; for(;;){printf("%s",prompt?prompt:"");fflush(stdout);int ok=get_line(line);if(!ok){printf("\n");return INPUT_QUIT;}if(ok<0){puts("Input too long.");continue;}char *v=trim(line);InputAction a=command(v);if(a!=INPUT_OK)return a;int n;if(!parse_int(v,&n)||n<min||n>max){printf("Enter a whole number between %d and %d.\n",min,max);continue;}*out=n;return INPUT_OK;} }
InputAction read_money(const char *prompt,int64_t *out) { if(!out)return INPUT_ERR_NULL; char line[BUFFER_SIZE]; for(;;){printf("%s",prompt?prompt:"");fflush(stdout);int ok=get_line(line);if(!ok){printf("\n");return INPUT_QUIT;}if(ok<0){puts("Input too long.");continue;}char *v=trim(line);InputAction a=command(v);if(a!=INPUT_OK)return a;if(!parse_money_cents(v,out)){puts("Enter a non-negative amount, e.g. 12.50.");continue;}return INPUT_OK;} }
InputAction read_nonempty_string(const char *prompt,char *out,size_t size) { if(!out||size<2)return INPUT_ERR_NULL;char line[BUFFER_SIZE];for(;;){printf("%s",prompt?prompt:"");fflush(stdout);int ok=get_line(line);if(!ok){printf("\n");return INPUT_QUIT;}if(ok<0){printf("Input too long (max %zu characters).\n",size-1);continue;}char *v=trim(line);InputAction a=command(v);if(a!=INPUT_OK)return a;if(!*v){puts("Value cannot be empty.");continue;}if(strchr(v,',')){puts("Commas are not allowed.");continue;}if(strlen(v)>=size){printf("Input too long (max %zu characters).\n",size-1);continue;}snprintf(out,size,"%s",v);return INPUT_OK;} }
