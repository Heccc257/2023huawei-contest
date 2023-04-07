# CXX=g++-8  # 指定编译器为 g++-8
CXX=g++-8  # 指定编译器为 g++-8

CXXFLAGS=-std=c++17 -Wall -Wextra  # 指定编译选项

SRC=main.cpp  # 源代码文件名

TARGET=main  # 目标可执行文件名

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)


.PHONY: clean

clean:
	rm -f $(TARGET)

# docker exec -i gcc8Compliler bash -c "cd root/ && make"