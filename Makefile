CROSS_COMPILE 	?=
CROSS_PREFIX	?=
CC		:= $(CROSS_COMPILE)gcc
CXX		:= $(CROSS_COMPILE)g++
CFLAGS 	?= -I. -g -O0 -std=gnu11
CXXFLAGS?= -I. -g -std=gnu++11 `pkg-config --cflags --libs opencv`
LDFLAGS	?=
LIBS	:= -larducam_mipicamera
OLIB	:= lib
examples:= preview capture video list_format capture_raw raw_callback read_write_sensor_reg ov9281_external_trigger preview-camera0 preview-dualcam capture-dualcam video2stdout capture2opencv qrcode_detection  
%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(examples)

preview: preview.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

capture: capture.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)
	
capture_raw: capture_raw.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

video: video.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

list_format: list_format.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

raw_callback: raw_callback.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

read_write_sensor_reg : read_write_sensor_reg.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

video2stdout : video2stdout.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

capture2opencv : capture2opencv.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

qrcode_detection: qrcode_detection.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS) -lzbar

preview-camera0 : preview-camera0.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

preview-dualcam : preview-dualcam.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

capture-dualcam : capture-dualcam.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

ov9281_external_trigger : ov9281_external_trigger.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)
	
clean:
	-rm -f *.o
	-rm -f $(examples)

.PHONY: install

install: 
	sudo install -m 644 $(OLIB)/libarducam_mipicamera.so /usr/lib/

