/*****************
 * monitoring_zimknives.h
 * 
 * 
 *****************/

#ifndef _MONITORING_ZIMKNIVES_H

extern TaskHandle_t xHandle_1;
extern TaskHandle_t xHandle_2;

/*
 * local function declarations
 */
void sample_process_1(void *pvParameters);
void sample_process_2(void *pvParameters);

#define _MONITORING_ZIMKNIVES_H
#endif