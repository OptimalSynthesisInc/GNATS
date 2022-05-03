#!/bin/sh

export GNATS_HOME=${PWD}

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$GNATS_HOME/dist:/usr/lib64

java -cp dist/gnats-standalone.jar:$GNATS_HOME/../GNATS_Client/dist/gnats-client.jar:$GNATS_HOME/../GNATS_Client/dist/gnats-shared.jar:$GNATS_HOME/../GNATS_Client/dist/json.jar:$GNATS_HOME/../GNATS_Server/dist/commons-logging-1.2.jar -Xmx768m TestRun $1
