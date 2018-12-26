SRCS:=main.cpp connection.cpp sql.cpp common.cpp
EXE:=chat_server

all:$(EXE)

$(EXE):$(SRCS)
	g++ -g -o $@ $^ \
	-I/usr/include/mysql \
	-L/usr/lib64/mysql \
	-lmysqlclient \
	-lz \
	-lcrypto

clean:
	rm -rf $(EXE)