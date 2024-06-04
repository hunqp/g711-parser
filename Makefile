CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11

TARGET = main

SOURCES = main.cpp g711.cpp

OBJECTS = $(SOURCES:.cpp=.o)

HEADERS = -I.

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)
	@echo "--- BUILD SUCCESS ---"

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)

run: all
	@./$(TARGET)

play-alaw:
	play -t raw -r 8k -e a-law -b 8 -c 1 audio_voices.aLaw

play-pcm:
	ffplay -f s16le -ar 8000 -ac 1 audio_voices.pcm

.PHONY: all clean run play
