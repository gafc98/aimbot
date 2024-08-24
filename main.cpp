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

#define MAX_SERVO 600
#define MIN_SERVO 100

float cap_servo(float a) // a must be between 0 and 1
{
    return a * (MAX_SERVO - MIN_SERVO) + MIN_SERVO;
}

int main(int argc, char* argv[])
{
    // get main i2c bus object
    I2C_BUS i2c_bus = I2C_BUS(0);
    PCA9685 pwm = PCA9685(&i2c_bus, 0x40);
    ADS1115 adc = ADS1115(&i2c_bus, 0x48);

    pwm.turn_off();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    pwm.set_PWM_freq(50);
    pwm.wake_up();

    float a0, a1;

    while (true)
    {
        adc.set_config(0);
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // unsure why  this delay is needed, but if I don't have it, it cant read
        a0 = adc.read_voltage() / 3.3; // to obtain percentage for servo 0
        adc.set_config(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // unsure why  this delay is needed, but if I don't have it, it cant read
        a1 = adc.read_voltage() / 3.3; // to obtain percentage for servo 1

        a0 = cap_servo(a0);
        a1 = cap_servo(a1);

        pwm.set_PWM(14, 0, a0);
        pwm.set_PWM(15, 0, a1);

        std::cout << "a0: " << a0 << "\t" << "a1: " << a1 << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

}