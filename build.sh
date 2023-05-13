gcc -O3 -I src/ src/indexTravInf.c -o bin/IndexTI
gcc  -I src/ src/server.c -L pthread -o bin/server
gcc -O3 -I src/ src/client.c -L pthread -o bin/client