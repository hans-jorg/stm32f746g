#ifndef TOUCH_H
#define TOUCH_H
/**
 * @file touch.h
 *
 * @brief Touch Screen interface
 *
 * @note
 *
 * @author Hans (hans@ele.ufes.br)
 * @version 0.1
 * @date 2023-05-28
 *
 * @copyright Copyright (c) 2023
 *
 */


typedef struct {
    uint16_t event;
    uint16_t id;
    uint16_t x;
    uint16_t y;
    uint16_t weight;
    uint16_t misc;
} Touch_Info;

int Touch_Init(void);
int Touch_Read(Touch_Info *touch);
int Touch_Detected(void);

#endif // TOUCH_H
