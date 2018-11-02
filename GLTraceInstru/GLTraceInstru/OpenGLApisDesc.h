#ifndef _OPENGLAPISDESC_H
#define _OPENGLAPISDESC_H
typedef struct ApiGroup_t
{
    const char* szName;
    int i_left;         //inclusive
    int i_right;        //exclusive
} ApiGroup;
extern const char* g_glApiNames[509];
extern ApiGroup g_apiGroup[14];
#endif