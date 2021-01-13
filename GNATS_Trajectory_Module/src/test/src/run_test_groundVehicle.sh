export NATS_SERVER_HOME=${PWD}

export LD_LIBRARY_PATH=$NATS_SERVER_HOME/dist

valgrind --leak-check=full --track-origins=yes --log-file=valgrind.log ./test_groundVehicle
