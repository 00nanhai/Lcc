#!/bin/bash
#

mkdir -p /usr/local/xFenguang

echo "remove old install.."
rm -rf /usr/local/xFenguang/new_lcc
rm -rf /etc/init.d/lccd

echo "install new file.."
mv new_lcc /usr/local/xFenguang
mv lccd /etc/init.d/
