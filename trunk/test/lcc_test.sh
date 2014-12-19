#! /bin/bash
ip=192.168.30.65
#ip=122.72.1.163
wurl=update.360safe.com/v3/safeup_lib64.cab
ckey=c49cc9a127e844247abb7f7ed1a8090b
curl=192.168.30.65:9090/data2/5/6/69/6/5ee17754b64795c06806d1becf566965.zip
host=$ip
size=222
hash=111
#name=A20140402_0001
name=B20140401
#name=reflector_list_update.txt

case $1 in
get_ckey)
FILE=get_ckey.html
HTTP="http://$ip:9000/lcc.php?operation=0&wurl=$wurl&tb_name=data"
;;
set_ckey)
echo "Nothing"
;;
get_curl)
FILE=get_curl.html
HTTP="http://$ip:9000/lcc.php?operation=2&ckey=$ckey&tb_name=data"
;;
set_curl)
FILE=set_curl.html
HTTP="http://$ip:9000/lcc.php?operation=3&wurl=$wurl&ckey=$ckey&curl=$curl&host=$host&size=$size&hash=$hash&tb_name=data"
;;
del_curl)
FILE=del_curl.html
HTTP="http://$ip:9000/lcc.php?operation=4&curl=$curl&hash=$hash&tb_name=data"
;;
dump_host)
FILE=dump_host.tgz
HTTP="http://$ip:9000/lcc.php?operation=5&host=$ip&tb_name=data"
;;
create_bfile)
FILE=create_bfile.html
HTTP="http://$ip:9000/lcc.php?operation=6"
;;
get_afile)
FILE=$name.tgz
HTTP="http://$ip:9000/lcc.php?operation=7&name=$name"
;;
get_bfile)
FILE=$name.tgz
HTTP="http://$ip:9000/lcc.php?operation=7&name=$name"
;;
update_list)
name=reflector_list_update.txt
FILE=update_list.txt
HTTP="http://$ip:9000/lcc.php?operation=7&name=$name"
;;
*)
echo "Nothing."
exit 0
;;
esac
curl -o $FILE --connect-timeout 10 --retry 1  -y 5 -Y 1 -s $HTTP
