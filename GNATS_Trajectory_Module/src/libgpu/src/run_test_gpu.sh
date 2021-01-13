#cp ../../../lib/lib*.so ../../../../GNATS_Server/dist/

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/cuda/targets/x86_64-linux/lib:../../../../GNATS_Server/dist

./test_gpu
