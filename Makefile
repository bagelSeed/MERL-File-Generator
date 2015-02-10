CXX = g++
CXXFLAGS = -Wall -MMD
EXEC = MERL_INPUT
OBJECTS = kind/kind.o lexer/lexer.o generator/relasm.o
DEPENDS = ${OBJECTS:.o=.d}

${EXEC}: ${OBJECTS}
	${CXX} ${CXXFLAGS} ${OBJECTS} -o ${EXEC}

-include ${DEPENDS}

.PHONY: clean

clean:
	rm ${OBJECTS} ${EXEC} ${DEPENDS}
