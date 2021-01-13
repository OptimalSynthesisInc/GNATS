export NATS_SERVER_HOME=${PWD}

export LD_LIBRARY_PATH=$NATS_SERVER_HOME/dist

./test_routeGenerator
