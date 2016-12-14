cc 			= g++ -std=c++11
deps        = unline.h
ob			= ocr.o ocr_test_driver.o
tests 		= ocr_test_driver
cflags 		= -Wall -Wextra -Werror -Wpedantic -g -O2
cv 			= `pkg-config --cflags --libs opencv`
tess 		= -llept -ltesseract -lssl -lcrypto

all: $(ob) tests
	$(cc) $(ob) -o smocr $(cv) $(tess)

tests: $(ob)
	$(cc) $(ob) -o ocr_test_driver $(cv) $(tess)

ocr.o: ocr.cc
	$(cc) $(cflags) -c ocr.cc

ocr_test_driver.o: $(tests).cc $(deps)
	$(cc) $(cflags) -c $(tests).cc

clean:
	rm -f *.o *.gch
