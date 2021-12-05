/* AUTOGENERATED FILE. DO NOT EDIT. */
#ifndef _MOCKSDC_CONTROL_H
#define _MOCKSDC_CONTROL_H

#include "sdc_control.h"

/* Ignore the following warnings, since we are copying code */
#if defined(__GNUC__) && !defined(__ICC) && !defined(__TMS470__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 6 || (__GNUC_MINOR__ == 6 && __GNUC_PATCHLEVEL__ > 0)))
#pragma GCC diagnostic push
#endif
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wpragmas"
#endif
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wduplicate-decl-specifier"
#endif

void Mocksdc_control_Init(void);
void Mocksdc_control_Destroy(void);
void Mocksdc_control_Verify(void);




#define sdc_control_init_Expect() sdc_control_init_CMockExpect(__LINE__)
void sdc_control_init_CMockExpect(UNITY_LINE_TYPE cmock_line);
#define sdc_control_open_Expect() sdc_control_open_CMockExpect(__LINE__)
void sdc_control_open_CMockExpect(UNITY_LINE_TYPE cmock_line);
#define sdc_control_close_Expect() sdc_control_close_CMockExpect(__LINE__)
void sdc_control_close_CMockExpect(UNITY_LINE_TYPE cmock_line);

#if defined(__GNUC__) && !defined(__ICC) && !defined(__TMS470__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 6 || (__GNUC_MINOR__ == 6 && __GNUC_PATCHLEVEL__ > 0)))
#pragma GCC diagnostic pop
#endif
#endif

#endif