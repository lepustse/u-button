# u-button
u for universal

## 1、介绍
u-button 是一款开源通用按键驱动库。有三大特点，一是该库采用读回调机制获取所有的按键状态，因而不受限于按键的类型，可支持独立按键，矩阵按键，以及AD按键等。二是采用事件回调机制上报按键事件，开发者只需要关注于自己的代码逻辑即可。第三，它支持中断模式，开发者可根据自己需求进行相应配置。

### 1.1、目录结构
```
.
├── examples
├── LICENSE
├── README.md
├── u-button.c
└── u-button.h
```
> examples 暂时为空，后期会补上

### 1.2、资源占用
待定

### 1.3、支持平台
rtos、裸机均可使用

### 1.4版本日志
- v1.0
	- u-button 初版

## 2、如何使用

### 2.1、数据介绍

#### 2.1.1、设备结构体
| 需要配置的成员          | 说明 |
|-------------------------|------|
| filter_time             |      |
| hold_active_time        |      |
| button_irq_enabled      |      |
| repeat_speed            |      |
| ub_device_read_callback |      |
| ub_event_callback       |      |
| ops                     |      |


#### 2.1.2、操作结构体
```C
struct u_button_ops {
    void (*ub_exti_irq_control_callback)(ub_bool_t enabled);
    void (*ub_time_irq_control_callback)(ub_bool_t enabled);
};

```

#### 2.1.3、按键事件
| 按键事件                     | 说明                               |
|------------------------------|------------------------------------|
| UB_EVENT_BUTTON_DOWN         | 按键按下，每次按下都会触发         |
| UB_EVENT_BUTTON_UP           | 按键抬起，每次抬起都会触发         |
| UB_EVENT_BUTTON_REPEAT_CLICK | 重复击事件，repeat变量记录连击次数 |
| UB_EVENT_BUTTON_HOLD         | 长按事件触发                       |

### 2.1、中断模式
选用了中断模式，需要将`button_irq_enabled`变量置1，并且设置ops回调函数，最后要将`ub_device_state_set();`放在exti中断处理函数中调用。

#### 2.1.1、u-button初始化
- 定义一个设备类型的全局变量

```
ub_dev_t g_dev;
```

- 依次给设备结构体赋值初始化

```C
	g_dev.filter_time = 0;								// 设置消抖时间
	g_dev.hold_active_time = 40;						// 设置长按有效时间
	g_dev.button_irq_enabled = UB_BUTTON_IRQ_USE;		// 设置中断模式
	g_dev.repeat_speed = 30;							// 设置重复击速度

	g_dev.ub_device_read_callback = funcA;				// 设置读回调
	g_dev.ub_event_callback = funcB;					// 设置事件回调
	g_dev.ops->ub_exti_irq_control_callback = funcC;	// 设置按键中断开关
	g_dev.ops->ub_time_irq_control_callback = funcD;	// 设置定时器中断开关

```
> 时间参数的配置与用户调用`ub_device_state_handle()`的频率有关

- 调用初始化接口

```
ub_device_init(&g_dev);
```

#### 2.1.2、状态设置
```
ub_device_state_set(&g_dev);
```

#### 2.1.3、周期性调用按键处理
```
ub_device_state_handle(&g_dev);
```

### 2.2、非中断模式

## 3、注意事项

## 4、联系方式

## 5、许可
