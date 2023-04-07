#!/bin/bash
# Ubuntu22.04中不支持g++-8,通过docker容器编译
docker restart gcc8Compliler
docker exec -i gcc8Compliler bash -c "cd root/ && make"