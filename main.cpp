#include <iostream>
#include <ctime>
#include <chrono>
#include <string.h>
#include <sstream>
#include "include/i2c_bus.cpp"
#include "include/ads1115.cpp"
#include "include/pca9685.cpp"
#include <thread>
#include <mutex>

int main(int argc, char* argv[])
{
    // get main i2c bus object
    I2C_BUS i2c_bus = I2C_BUS(0);
    PCA9685 pwm = PCA9685(&i2c_bus, 0x40);
    pwm.turn_off();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    pwm.set_PWM_freq(50);
    pwm.wake_up();

    __u8 pwmnum = 13; // pin number

    while (true)
    {
        for (__u16 i = 0; i < 4096; i += 8)
        {
            pwm.set_PWM(pwmnum, 0, i);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

}