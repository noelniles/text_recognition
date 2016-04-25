CC = g++ -std=c++11
CFLAGS = -Wall -Wextra -Werror -Wpedantic -g -O2
SEGMENT_TARGET = segment
CV = `pkg-config --cflags --libs opencv`
TESS = -llept -ltesseract

all: $(OCR_TARGET)

$(SEGMENT_TARGET): $(SEGMENT_TARGET).cc
	$(CC) $(CFLAGS) -o $(SEGMENT_TARGET) $(SEGMENT_TARGET).cc $(CV) $(TESS)
