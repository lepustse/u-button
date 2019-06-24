/*
 * File       : u-button.h
 *
 * Change Logs:
 * Date         Author      Notes
 * 2019-05-04   never       the first version
 */

#ifndef __U_BUTTON_H
#define __U_BUTTON_H

typedef int                     ub_err_t;
typedef int                     ub_bool_t;
typedef int                     ub_even_t;
typedef int                     ub_stat_t;
typedef unsigned int            ub_size_t;
typedef unsigned int            ub_tick_t;

#define UB_OK                   0
#define UB_ERR                  1

#define UB_EXTI_IRQ_ENABLE      1
#define UB_EXTI_IRQ_DISABLE     0
#define UB_TIME_IRQ_ENABLE      1
#define UB_TIME_IRQ_DISABLE     0

#define UB_EXTI_IRQ_USE         1
#define UB_EXTI_IRQ_NOT_USE     0

enum U_BUTTON_EVENT {
    UB_EVENT_NONE = 0,
    UB_EVENT_BUTTON_DOWN,
    UB_EVENT_BUTTON_UP,
    UB_EVENT_BUTTON_REPEAT_CLICK,
    UB_EVENT_BUTTON_HOLD,
};

struct u_button_ops {
    void (*ub_exti_irq_control_callback)(ub_bool_t enabled);
    void (*ub_time_irq_control_callback)(ub_bool_t enabled);
};

struct u_button_data {
    ub_even_t event;
    ub_tick_t ticks;
    ub_tick_t repeat;
    ub_stat_t state;
};

#define BUTTON_NUM_MAX          16

struct u_button_device {
    ub_tick_t filter_time; // 消抖时间
    ub_tick_t hold_active_time; // 长按有效时间
    ub_bool_t button_irq_enabled;
    ub_tick_t repeat_speed; // n击速度

    struct u_button_data btn[BUTTON_NUM_MAX];

    uint32_t (*ub_device_read_callback)(void);
    void (*ub_event_callback)(struct u_button_device *dev);
    struct u_button_ops ops;
};
typedef struct u_button_device ub_dev_t;

#ifdef __cplusplus
extern "C" {

ub_err_t ub_device_init(ub_dev_t *dev);
void ub_device_state_set(ub_dev_t *dev);
void ub_device_state_handle(ub_dev_t *dev);

#endif
#ifdef __cplusplus
}
#endif
#endif /* __U_BUTTON_H */
