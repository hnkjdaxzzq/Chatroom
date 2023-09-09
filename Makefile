server: 
	clang++ client.cpp src/util.cpp -std=c++2a -o client
	clang++ -std=c++2a -pthread -g \
	server.cpp \
	src/util.cpp src/Epoll.cpp src/InetAddress.cpp src/Socket.cpp \
	src/Rio.cpp src/Connection.cpp \
	src/Channel.cpp src/EventLoop.cpp src/Server.cpp src/Acceptor.cpp \
	src/ThreadPool.cpp \
	-o server
clean:
	rm server 
	rm client