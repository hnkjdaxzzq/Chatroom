server: 
	g++ client.cpp src/util.cpp -o client
	g++ server.cpp src/util.cpp src/InetAddress.cpp src/Socket.cpp -o server

clean:
	rm server 
	rm client