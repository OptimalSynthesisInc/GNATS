export LD_LIBRARY_PATH=/home/oliverchen/Projects_NATS/NATS_Server/lib

valgrind --leak-check=full --track-origins=yes --log-file=valgrind.log ./test_monte_carlo
