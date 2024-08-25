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
#include <vector>

#include <opencv2/opencv.hpp>
// install as:
// sudo apt-get install -y libopencv-dev
// sudo ln -s /usr/include/opencv4/opencv2 /usr/include/opencv2


#define MAX_SERVO 600
#define MIN_SERVO 100

float cap_servo(float a) // a must be between 0 and 1
{
    return a * (MAX_SERVO - MIN_SERVO) + MIN_SERVO;
}

void knob_mode()
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

void square()
{
    // get main i2c bus object
    I2C_BUS i2c_bus = I2C_BUS(0);
    PCA9685 pwm = PCA9685(&i2c_bus, 0x40);

    pwm.turn_off();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    pwm.set_PWM_freq(50);
    pwm.wake_up();

    pwm.set_PWM(15, 0, 331);
    while (true)
    {
        for (__u16 a0 = 393; a0 >= 329; a0--)
        {
            pwm.set_PWM(14, 0, a0);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        for (__u16 a1 = 338; a1 <= 350; a1++)
        {
            pwm.set_PWM(15, 0, a1);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        for (__u16 a0 = 329; a0 <= 393; a0++)
        {
            pwm.set_PWM(14, 0, a0);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        for (__u16 a1 = 350; a1 >= 338; a1--)
        {
            pwm.set_PWM(15, 0, a1);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

void point_to_coordinates(__u16 a0, __u16 a1)
{
    I2C_BUS i2c_bus = I2C_BUS(0);
    PCA9685 pwm = PCA9685(&i2c_bus, 0x40);

    pwm.set_PWM_freq(50);
    pwm.wake_up();

    pwm.set_PWM(14, 0, a0);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    pwm.set_PWM(15, 0, a1);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}


int test_cv22() {
    using namespace cv;
    using namespace std;
    VideoCapture cap(0); // Open webcam device 0

    if (!cap.isOpened()) {
        cerr << "Error opening video capture" << endl;
        return -1;
    }

    // Create a cascade classifier for hand detection
    CascadeClassifier hand_cascade;
    if (!hand_cascade.load("haarcascade_hand.xml")) {
        cerr << "Error loading hand cascade" << endl;
        return -1;
    }

    while (true) {
        Mat frame;
        cap >> frame;

        if (frame.empty()) {
            break;
        }

        // Convert to grayscale for hand detection
        Mat gray;
        cvtColor(frame, gray, COLOR_BGR2GRAY);

        // Detect hands
        vector<Rect> hands;
        hand_cascade.detectMultiScale(gray, hands, 1.1, 2);

        // Draw circles around detected hands and their centers
        for (size_t i = 0; i < hands.size(); i++) {
            Rect hand = hands[i];
            Point center(hand.x + hand.width / 2, hand.y + hand.height / 2);
            circle(frame, center, 5, Scalar(0, 255, 0), 2);
            rectangle(frame, hand, Scalar(0, 0, 255), 2);
        }

        imshow("Frame", frame);

        if (waitKey(1) == 27) {
            break;
        }
    }

    cap.release();
    destroyAllWindows();

    return 0;
}

void test_cv2()
{
    using namespace cv;
    using namespace std;

    VideoCapture cap(0); // Open webcam device 0

    if (!cap.isOpened()) {
        std::cerr << "Error opening video capture" << std::endl;
        return;
    }

    // Create a window with a specific size
    namedWindow("Frame", WINDOW_NORMAL);
    resizeWindow("Frame", 640, 360); // Adjust the width and height as needed

    while (true) {
        Mat frame;
        cap >> frame;

        if (frame.empty()) {
            break;
        }

        // Convert to HSV color space
        Mat hsv;
        cvtColor(frame, hsv, COLOR_BGR2HSV);

        // Define ranges for red color in HSV
        Scalar lower_red(150, 100, 100);
        Scalar upper_red(180, 255, 255);

        // Threshold the HSV image to get only red colors
        Mat mask;
        inRange(hsv, lower_red, upper_red, mask);

        // Find contours in the mask
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // Find the contour with the largest area
        int largestContourIndex = -1;
        float largestContourArea = 0;
        for (size_t i = 0; i < contours.size(); i++) {
            float contourArea = cv::contourArea(contours[i]);
            if (contourArea > largestContourArea) {
                largestContourIndex = i;
                largestContourArea = contourArea;
            }
        }

        // Draw a circle around the largest contour, if it exists
        if (largestContourIndex != -1) {
            vector<Point> largestContour = contours[largestContourIndex];
            Rect rect = boundingRect(largestContour);
            Point center(rect.x + rect.width / 2, rect.y + rect.height / 2);
            circle(frame, center, 5, Scalar(0, 255, 0), 2);
        }

        imshow("Frame", frame);

        if (waitKey(1) == 27) {
            break;
        }
    }

    cap.release();
    destroyAllWindows();
}

int main(int argc, char* argv[])
{
    test_cv22();

    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <integer1> <integer2>" << std::endl;
        square();
    }

    
    point_to_coordinates(std::stoi(argv[1]), std::stoi(argv[2]));
    
}