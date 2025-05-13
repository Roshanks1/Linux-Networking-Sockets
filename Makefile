#MakeFile for echo_s

CXX = g++
CXXFLAGS = -Wall -g

all : echo_s echo_c log_s

# === Server ===
echo_s: echo_s.o
	$(CXX) $(CXXFLAGS) -o echo_s echo_s.o

echo_s.o: echo_s.cpp
	$(CXX) $(CXXFLAGS) -c echo_s.cpp -o echo_s.o

# === Client ===
echo_c: echo_c.o
	$(CXX) $(CXXFLAGS) -o echo_c echo_c.o

echo_c.o: echo_c.cpp
	$(CXX) $(CXXFLAGS) -c echo_c.cpp -o echo_c.o

# === Log ===
log_s: log_s.o
	$(CXX) $(CXXFLAGS) -o log_s log_s.o

log_s.o: log_s.cpp
	$(CXX) $(CXXFLAGS) -c log_s.cpp -o log_s.o

# === Clean ===
clean:
	rm -f echo_s echo_c log_s echo.log *.o
