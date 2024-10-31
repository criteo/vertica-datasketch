#!/bin/bash
docker build -t vertica-builder-ds:local .

rm -rf build
mkdir -p build
docker kill vbuilder-datask || true
sleep 1
docker rm vbuilder-datask || true
sleep 1
docker run --rm --name vbuilder-datask -v=$PWD/SOURCES:/sources -v=$PWD/build:/build -v=/opt/vertica/sdk:/opt/vertica/sdk  -d vertica-builder-ds:local sleep 3600
echo "Run docker exec -ti vbuilder-datask bash # now"
echo "Then, you may run the following to build"
echo "cmake /sources; make clean; make"