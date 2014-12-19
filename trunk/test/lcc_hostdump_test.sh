#!/bin/bash
tmp=./dumphost.tgz
#http="\'"
http="http://122.72.1.163:9000/lcc.php?operation=5&host=111.111.111.111&tb_name=data"
#option= ""
curl -o $tmp --connect-timeout 10 --retry 1  -y 5 -Y 1 -s $http
