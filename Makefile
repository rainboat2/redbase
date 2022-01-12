CC=g++
LDFLAGS=
CPPFLAGS=-std=c++14 -g
build=./build

pf_sources=PF_PageHandle.cpp PF_PrintError.cpp PF_FileHandle.cpp PF_Manager.cpp PF_BufferManger.cpp

sources=main.cpp ${pf_sources}
objs=${sources:cpp=o}

redbase: ${sources} ${pf_head}
	g++ ${LDFLAGS} ${CPPFLAGS} ${sources} -o $@

${objs}: %.o: %.cpp
	${CC} -MM -c ${CPPFLAGS} $< -o ${build}/$@

.PHONY: clean print
clean:
	rm redbase
print:
	echo ${objs}