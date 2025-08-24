GCC = arm-linux-gnueabihf-gcc 
CFLAGS = -Wall
TARGET = led
SRCS = led.c 

$(TARGET): $(SRCS)
	$(GCC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)