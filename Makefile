CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

VulkanDigicam: *.cpp *.hpp
	g++ $(CFLAGS) -o VulkanDigicam *.cpp $(LDFLAGS)

.PHONY: test clean

test: VulkanDigicam
	./VulkanDigicam

clean:
	rm -f VulkanDigicam