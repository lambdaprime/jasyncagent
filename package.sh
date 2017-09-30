#! /bin/sh

CWD=$(pwd)/$(dirname $0)

cd $CWD
rm -rf release
mkdir -p release/jasyncagent/debug
cp README.org release/jasyncagent
cp libjasyncagentpreload/libjasyncagentpreload/Release/libjasyncagentpreload.so release/jasyncagent
cp libjasyncagentpreload/libjasyncagentpreload/Debug/libjasyncagentpreload.so release/jasyncagent/debug
cp libjasyncagent/Release/libjasyncagent.so release/jasyncagent
cp libjasyncagent/Debug/libjasyncagent.so release/jasyncagent/debug
cp jasyncagent/release/jasyncagent.jar release/jasyncagent
cd release
zip -9r jasyncagent.zip jasyncagent 
