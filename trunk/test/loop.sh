#!/bin/bash
if [ $1!="show_res" ];then 
i=1
for ((i=1; i <= 1; i++));
do
nohup ./lcc_hostdump_test.sh $i &
done #for end
else #else
for ((i=1; i <= 1; i++));
do
echo "------ 1 ------"
ls $i_*.tgz | wc -l
done #for end
fi
#endif
