import cv2
import numpy as np
import os
import time
import mediapipe as mp

# Initialize MediaPipe hands
mp_hands = mp.solutions.hands
hands = mp_hands.Hands()
mp_draw = mp.solutions.drawing_utils

# Define the range of red colors in HSV
lower_red = np.array([150, 100, 100])
upper_red = np.array([180, 255, 255])

# Open the webcam on device 1
cap = cv2.VideoCapture(0)

last_cx = 0
last_cy = 0
palm_center_x = 495
palm_center_y = 248

os.system("sudo ./aimbot 329 338")
time.sleep(1)
os.system("sudo ./aimbot 393 338")
time.sleep(1)
os.system("sudo ./aimbot 393 350")
time.sleep(1)
os.system("sudo ./aimbot 329 350")
time.sleep(1)

ctrl_x = 329
ctrl_y = 338

while True:
    # Read a frame from the webcam
    ret, frame = cap.read()

    # Convert the BGR image to RGB.
    image = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    # Process the image with MediaPipe Hands.
    results = hands.process(image)

    # Convert the frame to HSV color space
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Create a mask for the red color range
    mask = cv2.inRange(hsv, lower_red, upper_red)

    # Find contours in the mask
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    # Sort contours by area in descending order
    #contours = sorted(contours, key=cv2.contourArea, reverse=True)
    #print(len(contours))

    current_distance = float('inf')
    for contour in contours:
        x, y, w, h = cv2.boundingRect(contour)
        cx, cy = x + w // 2, y + h // 2
        distance = (cx - last_cx) ** 2 + (cy - last_cy) ** 2
        if distance < current_distance:
            current_distance = distance
            current_cx, current_cy, radius = cx, cy, int(max(w, h) / 2)


    # Draw a circle around the largest contour (assuming it's the laser pointer)
    if len(contours) > 0:
        cv2.circle(frame, (current_cx, current_cy), radius, (0, 0, 255), 2)
        last_cx, last_cy = current_cx, current_cy

    if results.multi_hand_landmarks:
        for hand_landmarks in results.multi_hand_landmarks:

            # Get the center of the palm
            palm_center_x, palm_center_y = 0, 0
            for id, lm in enumerate(hand_landmarks.landmark):
                # Use only the palm landmarks for center calculation
                if id in [0, 5, 9, 13, 17]:
                    x, y, z = int(lm.x * frame.shape[1]), int(lm.y * frame.shape[0]), lm.z
                    palm_center_x += x
                    palm_center_y += y
            palm_center_x //= 5
            palm_center_y //= 5

            # Draw a red dot at the palm center
            cv2.circle(frame, (palm_center_x, palm_center_y), 5, (0, 0, 255), -1)

    # Display the frame
    # cv2.imshow('Frame', frame)

    if len(contours) > 0 and results.multi_hand_landmarks:
        ctrl_x += -(palm_center_x - last_cx) * 0.01
        ctrl_y += -(palm_center_y - last_cy) * 0.01

    print(f"{int(ctrl_x)}  {int(ctrl_y)}  {palm_center_x} <- {last_cx}, {palm_center_y} <- {last_cy}")

    os.system(f"sudo ./aimbot {int(ctrl_x)} {int(ctrl_y)}")

    # Check for keyboard input to exit
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release the webcam and close windows
cap.release()
cv2.destroyAllWindows()