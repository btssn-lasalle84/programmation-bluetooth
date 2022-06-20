# NE PAS TOUCHER
CFLAGS=-std=c++11 -Wall 
CXX=g++ $(CFLAGS) -c 
LD=g++ -o
LDFLAGS=-lbluetooth

TARGET_CLIENT = client-bluetooth.out
TARGET_SERVEUR = serveur-bluetooth.out
TARGETS = $(TARGET_CLIENT) $(TARGET_SERVEUR)

all: $(TARGETS)

$(patsubst %.out, %, $(TARGET_CLIENT)): $(TARGET_CLIENT)
	./$(TARGET_CLIENT)

$(TARGET_CLIENT): $(patsubst %.out, %.o, $(TARGET_CLIENT))
	$(LD) $@ $^ $(LDFLAGS)

$(patsubst %.out, %.o, $(TARGET_CLIENT)): $(patsubst %.out, %.cpp, $(TARGET_CLIENT))
	$(CXX) $< -o $@

$(patsubst %.out, %, $(TARGET_SERVEUR)): $(TARGET_SERVEUR)
	./$(TARGET_SERVEUR)

$(TARGET_SERVEUR): $(patsubst %.out, %.o, $(TARGET_SERVEUR))
	$(LD) $@ $^ $(LDFLAGS)

$(patsubst %.out, %.o, $(TARGET_SERVEUR)): $(patsubst %.out, %.cpp, $(TARGET_SERVEUR))
	$(CXX) $< -o $@

.PHONY: clean

clean:
	rm -f $(TARGETS) *.o *.*~

rebuild: clean all
