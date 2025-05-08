CXX = g++ # compilador
CXXFLAGS = -std=c++11 -Wall -O2 # flags

TARGET = prim # algoritmo
SRC = prim.cpp # codigo
OBJ = $(SRC:.cpp=.o) # objeto

$(TARGET): $(OBJ) # regras base de compilacao
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean: # apagar arquivos criados
	rm -f $(OBJ) $(TARGET)

run: $(TARGET) # regras de execucao do programa
	./$(TARGET) $(ARGS)