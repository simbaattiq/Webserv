SRCS=main.cpp srcs/Parser.cpp srcs/Server.cpp
OBJS=${SRCS:.cpp=.o}
OUT=server
CXX=c++
CXXFLAGS= -Wall -Wextra -Werror -std=c++98  -fsanitize=address
RM= rm -f

All : ${OUT}

${OUT} : ${OBJS}
	${CXX} ${CXXFLAGS} ${OBJS} -o ${OUT}

clean:
	${RM} ${OBJS}
fclean : clean
	${RM} ${OUT}
re : fclean All

.PHONY: make re clean fclean