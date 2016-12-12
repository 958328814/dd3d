#
#	Make file for DreamDota 3 Server
#

ifeq ($(mode),release)
CFLAGS = -O2 -Wall -I/usr/local/include -I/usr/include/mysql -I/usr/local/ssl/include
endif

ifeq ($(mode),beta)
CFLAGS = -g -pg -Wall -I/usr/local/include -I/usr/include/mysql -I/usr/local/ssl/include -DBETA
endif

ifeq ($(mode),)
mode = debug
CFLAGS = -g -pg -Wall -I/usr/local/include -I/usr/include/mysql -I/usr/local/ssl/include -DDEBUG
endif

CC = g++
LIBS = -lzmq -L/usr/local/lib/mysql -L/usr/local/ssl/lib -lssl -lcrypto -lmysqlpp -luuid -ltcmalloc -lconfig
SRC_PATH = src/
OUT_PATH = bin/


# entry
all: dd3d.o database.o connection.o crypt.o log.o utils.o message.o m_user.o m_offsets.o m_service.o c_login.o c_handshake.o c_offsets.o config.o c_alive.o c_update.o c_logout.o
	$(CC) $(CFLAGS) -o $(OUT_PATH)dd3d  $(LIBS) \
		dd3d.o database.o connection.o crypt.o utils.o message.o log.o m_user.o m_offsets.o m_service.o c_login.o c_handshake.o c_offsets.o config.o c_alive.o c_update.o c_logout.o

install:
	mkdir -p /etc/dd3d
	cp -r ./bin/* /etc/dd3d
cpremote:
	make clean
	scp -r . root@ticklog.me:/app/
	ssh root@ticklog.me

dd3d.o: $(SRC_PATH)dd3d.cpp $(SRC_PATH)dd3d.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)dd3d.cpp -o dd3d.o


# Database Module
database.o: $(SRC_PATH)database.cpp $(SRC_PATH)database.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)database.cpp -o database.o


# Connection Module
connection.o: $(SRC_PATH)connection.cpp $(SRC_PATH)connection.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)connection.cpp -o connection.o

# Config Module
config.o: $(SRC_PATH)config.cpp $(SRC_PATH)config.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)config.cpp -o config.o

# Log Module
log.o: $(SRC_PATH)log.cpp $(SRC_PATH)log.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)log.cpp -o log.o

# Crypt Module
crypt.o: $(SRC_PATH)crypt.cpp $(SRC_PATH)crypt.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)crypt.cpp -o crypt.o

# Utils Module
utils.o: $(SRC_PATH)utils.cpp $(SRC_PATH)utils.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)utils.cpp -o utils.o

# User Model
m_user.o: $(SRC_PATH)m_user.cpp $(SRC_PATH)m_user.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)m_user.cpp -o m_user.o

# Offsets Model
m_offsets.o: $(SRC_PATH)m_offsets.cpp $(SRC_PATH)m_offsets.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)m_offsets.cpp -o m_offsets.o

# Service Model
m_service.o: $(SRC_PATH)m_service.cpp $(SRC_PATH)m_service.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)m_service.cpp -o m_service.o

# Message
message.o: $(SRC_PATH)message.cpp $(SRC_PATH)message.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)message.cpp -o message.o


# Login Controller
c_login.o: $(SRC_PATH)c_login.cpp $(SRC_PATH)c_login.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)c_login.cpp -o c_login.o

# Logout Controller
c_logout.o: $(SRC_PATH)c_logout.cpp $(SRC_PATH)c_logout.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)c_logout.cpp -o c_logout.o

# Handshake Controller
c_handshake.o: $(SRC_PATH)c_handshake.cpp $(SRC_PATH)c_handshake.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)c_handshake.cpp -o c_handshake.o

# Offsets Controller
c_offsets.o: $(SRC_PATH)c_offsets.cpp $(SRC_PATH)c_offsets.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)c_offsets.cpp -o c_offsets.o

# Alive Controller
c_alive.o: $(SRC_PATH)c_alive.cpp $(SRC_PATH)c_alive.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)c_alive.cpp -o c_alive.o

# Update Controller
c_update.o: $(SRC_PATH)c_update.cpp $(SRC_PATH)c_update.h
	$(CC) $(CFLAGS) -c $(SRC_PATH)c_update.cpp -o c_update.o

#	Tests
test: test_server.o test_client.o test_openssl.o test_mysqlpp.o
	$(CC) $(CFLAGS) -o $(OUT_PATH)test_server test_server.o $(LIBS)
	$(CC) $(CFLAGS) -o $(OUT_PATH)test_client test_client.o $(LIBS)
	$(CC) $(CFLAGS) -o $(OUT_PATH)test_openssl test_openssl.o $(LIBS)
	$(CC) $(CFLAGS) -o $(OUT_PATH)test_mysqlpp test_mysqlpp.o $(LIBS)

test_server.o : $(SRC_PATH)test_server.cpp
	$(CC) $(CFLAGS) -c $(SRC_PATH)test_server.cpp -o test_server.o

test_client.o : $(SRC_PATH)test_client.cpp
	$(CC) $(CFLAGS) -c $(SRC_PATH)test_client.cpp -o test_client.o

test_openssl.o : $(SRC_PATH)test_openssl.cpp
	$(CC) $(CFLAGS) -c $(SRC_PATH)test_openssl.cpp -o test_openssl.o

test_mysqlpp.o : $(SRC_PATH)test_mysqlpp.cpp
	$(CC) $(CFLAGS) -c $(SRC_PATH)test_mysqlpp.cpp -o test_mysqlpp.o

clean:
	rm -f *.o
