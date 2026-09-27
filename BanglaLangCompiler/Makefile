CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra

all: banglacc

banglacc: src/main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

sample: banglacc
	./banglacc examples/hello.bng -o examples/hello.py
	python3 examples/hello.py

clean:
	rm -f banglacc examples/*.py
