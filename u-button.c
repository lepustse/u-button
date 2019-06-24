/*
 * File       : u-button.c
 *
 * Change Logs:
 * Date         Author      Notes
 * 2019-05-04   never       the first version
 */

#include <stdio.h>
#include "u-button.h"

static ub_dev_t g_dev;

enum U_BUTTON_STATE {
    UB_STATE_INIT = 0,
    UB_STATE_UNSTABLE,
    UB_STATE_STABLE,
    UB_STATE_WAIT_BUTTON_UP,
    UB_STATE_WAIT_BUTTON_DOWN_AGAIN,
};

static void __ub_dev_state_reset(ub_dev_t *dev) {
    uint32_t cnt = 0;
    uint32_t i, dev_read, tmp = 0;

    if (dev->ub_device_read_callback != NULL) {
        dev_read = dev->ub_device_read_callback();
    }

    for (i = 0; i < BUTTON_NUM_MAX; i++) {
        if ((dev_read & (1 << i)) == 0) { // 没有按下
            cnt++;
        }

        dev->btn[i].event = UB_EVENT_NONE;
        dev->btn[i].state = UB_STATE_INIT;
        dev->btn[i].ticks = 0;
        dev->btn[i].repeat = 0;
    }

    if (cnt == BUTTON_NUM_MAX) {
        if (dev->ops.ub_time_irq_control_callback != NULL) {
            dev->ops.ub_time_irq_control_callback(UB_TIME_IRQ_DISABLE);
        }

        if (dev->ops.ub_exti_irq_control_callback != NULL) {
            dev->ops.ub_exti_irq_control_callback(UB_EXTI_IRQ_ENABLE);
        }
    }
}

/**
 * This is the core of the universal button driver, please call is periodically.
 *
 * @param dev
 */
void ub_device_state_handle(ub_dev_t *dev) {
    uint32_t i, dev_read = 0;

    if (dev == NULL) {
        return;
    }

    if (dev->ub_device_read_callback != NULL) {
        dev_read = dev->ub_device_read_callback();
    }

    for (i = 0; i < BUTTON_NUM_MAX; i++) {

        if (dev->btn[i].state > 0) {
            dev->btn[i].ticks++;
        }

        switch (dev->btn[i].state) {
            case UB_STATE_INIT:
                if (dev_read & (1 << i)) {
                    dev->btn[i].state = UB_STATE_UNSTABLE;
                }
                break;

            case UB_STATE_UNSTABLE:
                if (dev_read & (1 << i)) { // button down
                    if (dev->ub_event_callback != NULL) {
                        dev->btn[i].event = UB_EVENT_BUTTON_DOWN;
                        dev->ub_event_callback(dev);
                    }

                    dev->btn[i].state = UB_STATE_STABLE;
                    dev->btn[i].repeat++;
                    if (dev->btn[i].repeat > 1) {
                        if (dev->ub_event_callback != NULL) {
                            dev->ub_event_callback(dev);
                        }
                    }
                } else {
                    if (dev->btn[i].repeat == 0) {
                        __ub_dev_state_reset(dev);
                    } else {
                        dev->btn[i].state = UB_STATE_WAIT_BUTTON_DOWN_AGAIN;
                    }
                }
                break;

            case UB_STATE_STABLE:
                if ((dev_read & (1 << i)) == 0) { // up
                    if (dev->ub_event_callback != NULL) {
                        dev->btn[i].event = UB_EVENT_BUTTON_UP;
                        dev->ub_event_callback(dev);
                    }
                    dev->btn[i].state = UB_STATE_WAIT_BUTTON_DOWN_AGAIN;
                } else {
                    // todo: 某key长按，其他key动作，key还要输出吗？
                    if (dev->btn[i].ticks > dev->hold_active_time) {
                        if (dev->ub_event_callback != NULL) {
                            dev->btn[i].event = UB_EVENT_BUTTON_HOLD;
                            dev->ub_event_callback(dev);
                        }
                        dev->btn[i].state = UB_STATE_WAIT_BUTTON_UP;
                    }
                }
                break;

            case UB_STATE_WAIT_BUTTON_UP:
                if ((dev_read & (1 << i)) == 0) {
                    if (dev->ub_event_callback != NULL) {
                        dev->btn[i].event = UB_EVENT_BUTTON_UP;
                        dev->ub_event_callback(dev);
                    }
                    __ub_dev_state_reset(dev);
                }
                break;

            case UB_STATE_WAIT_BUTTON_DOWN_AGAIN:
                if (dev->btn[i].ticks > dev->repeat_speed) {
                    // 超时了，应该输出单击，双击，返回init了
                    if (dev->ub_event_callback != NULL) {
                        dev->btn[i].event = UB_EVENT_BUTTON_REPEAT_CLICK;
                        dev->ub_event_callback(dev);
                    }
                    __ub_dev_state_reset(dev);
                } else {
                    dev->btn[i].state = UB_STATE_UNSTABLE;
                }
                break;

            default:
                break;
        }
    }
}

/**
 * This function is to set the unstable state(UB_STATE_UNSTABLE)
 * of the state machine. If you use interrupt mode(button_irq_enabled set to 1),
 * please put this function in the interrupt handler, otherwise ignore it.
 *
 * @param dev
 */
void ub_device_state_set(ub_dev_t *dev) {
    uint8_t i, tmp;
    uint32_t dev_read = 0;

    if (dev->ub_device_read_callback != NULL) {
        dev_read = dev->ub_device_read_callback();
    }

    for (i = 0; i < BUTTON_NUM_MAX; i++) {
        if (dev_read & (1 << i)) { // 判断有无按键按下
            dev->btn[i].state = UB_STATE_UNSTABLE;
            dev->btn[i].ticks = 0;
            dev->btn[i].repeat = 0;
        }
    }

    if (dev->ops.ub_exti_irq_control_callback != NULL) {
        dev->ops.ub_exti_irq_control_callback(UB_EXTI_IRQ_DISABLE);
    }

    if (dev->ops.ub_time_irq_control_callback != NULL) {
        dev->ops.ub_time_irq_control_callback(UB_TIME_IRQ_ENABLE);
    }
}

/**
 * This is the initialzation function of the universal button.
 *
 * @param dev
 *
 * @return the operation status, UB_OK on successful
 */
ub_err_t ub_device_init(ub_dev_t *dev) {
    uint32_t i;

    if (dev == NULL) {
        goto __exit;
    }

    g_dev.filter_time = dev->filter_time;
    g_dev.hold_active_time = dev->hold_active_time;
    g_dev.repeat_speed = dev->repeat_speed;
    g_dev.button_irq_enabled = dev->button_irq_enabled;

    for (i = 0; i < BUTTON_NUM_MAX; i++) {
        g_dev.btn[i].event = UB_EVENT_NONE;
        g_dev.btn[i].ticks = 0;
        g_dev.btn[i].repeat = 0;
        g_dev.btn[i].state = UB_STATE_INIT;
    }

    if (g_dev.button_irq_enabled) {
        g_dev.ops.ub_exti_irq_control_callback = dev->ops.ub_exti_irq_control_callback;
        g_dev.ops.ub_time_irq_control_callback = dev->ops.ub_time_irq_control_callback;
    } else {
        g_dev.ops.ub_exti_irq_control_callback = NULL;
        g_dev.ops.ub_time_irq_control_callback = NULL;
    }

    g_dev.ub_event_callback = dev->ub_event_callback;

    return UB_OK;

__exit:
    return -UB_ERR;
}
