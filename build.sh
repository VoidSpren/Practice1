gcc -O3 -I src/ src/indexTravInf.c -o bin/IndexTI
gcc -O3 -I src/ src/server.c -L pthread -o bin/server