# smartCar
stm32 smart car

GPIO PIN引脚占用情况:
# PA15                    Buzzer
PA15                    DHT11
PA8                     LED
PA0 PA1 PA4 PA5 PA6 PA7 Motor(PWM TIM2)
PA11                    Servo(PWM TIM1)
PA9 PA10                Bluetooth(USART1)
PA2 PA3 PA12            WIFI(USART2,TIM4,DMA)

PB6 PB7 PB8 PB9         OLED
PB12 PB13 PB14          Tracking
# PB0 PB1                 Ultrasonic(TIM3)
PC15                    IR_NEC(TIM3)
PB10 PB11               VoiceIdentify(USART3)
