PLUGIN_NAME=monitor-workspace-rules
SOURCE_FILES=$(wildcard ./*.cpp)

all: $(PLUGIN_NAME).so

$(PLUGIN_NAME).so: $(SOURCE_FILES)
	g++ -shared -Wall -fPIC --no-gnu-unique $(SOURCE_FILES) -g  -DWLR_USE_UNSTABLE `pkg-config --cflags pixman-1 libdrm hyprland` -std=c++23 -o $(PLUGIN_NAME).so

clean:
	rm -f ./$(PLUGIN_NAME).so
