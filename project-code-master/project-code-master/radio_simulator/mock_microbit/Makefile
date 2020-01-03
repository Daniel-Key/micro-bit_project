CC=g++
FLAGS=-Wall -Wextra -g

mock_mesh: clean MeshProtocol.o MicroBit.o main.o Type.o Radio.o Encryption.o
	${CC} ${FLAGS} -o mock_mesh MeshProtocol.o MicroBit.o main.o Type.o Radio.o Encryption.o -pthread

mock_nomesh: clean NomeshProtocol.o MicroBit.o main.o Type.o
	${CC} ${FLAGS} -o mock_nomesh NomeshProtocol.o MicroBit.o main.o Type.o -pthread

mock_dynamic: clean DynamicMeshProtocol.o MicroBit.o main.o Type.o
	${CC} ${FLAGS} -o mock_dynamic DynamicMeshProtocol.o MicroBit.o main.o Type.o -pthread

NomeshProtocol.o: ../../microbit/Protocol/NomeshProtocol.cpp Protocol.h
	cp ../../microbit/Protocol/NomeshProtocol.cpp NomeshProtocol.cpp
	${CC} ${FLAGS} -c -o NomeshProtocol.o NomeshProtocol.cpp

MeshProtocol.o: ../../microbit/source/Protocol.cpp Protocol.h
	cp ../../microbit/source/Protocol.cpp MeshProtocol.cpp
	cp ../../microbit/source/Encryption.h Encryption.h
	cp ../../microbit/source/Encryption.cpp Encryption.cpp
	${CC} ${FLAGS} -c -o MeshProtocol.o MeshProtocol.cpp

DynamicMeshProtocol.o: ../../microbit/Protocol/DynamicMeshProtocol.cpp Protocol.h
	cp ../../microbit/Protocol/DynamicMeshProtocol.cpp DynamicMeshProtocol.cpp
	cp ../../microbit/Protocol/DynamicMeshProtocol.h DynamicMeshProtocol.h
	${CC} ${FLAGS} -c -o DynamicMeshProtocol.o DynamicMeshProtocol.cpp


Protocol.h: ../../microbit/source/Protocol.h
	cp ../../microbit/source/Protocol.h Protocol.h

Encryption.o:
	${CC} ${FLAGS} -c -o Encryption.o Encryption.cpp

main.o:
	${CC} ${FLAGS} -c -o main.o main.cpp -pthread


.PHONY: clean run copy-mesh
clean:
	rm -f *.o mock_dynamic mock_nomesh mock_mesh NomeshProtocol.cpp
	rm -f MeshProtocol.cpp Protocol.h
	rm -f Encryption.h Encryption.cpp

copy-mesh: mock_mesh
	cp mock_mesh ../../website/data
