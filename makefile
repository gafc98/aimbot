aimbot: main.cpp
	g++ -fdiagnostics-color=always -g main.cpp -Ofast -fconcepts-ts -std=c++17 -o aimbot -I /usr/include -L /usr/lib -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_videoio -lopencv_objdetect

clean:
	rm aimbot