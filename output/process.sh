#!/bin/bash

step=300

echo 'process.sh can generate:'
echo '  tasks.R: Entire task log for RT tasks'
echo '  tasks.N: Entire task log for NRT tasks'
echo '  power: Total system power log'
echo '  taskcost: Total cost of all tasks'
echo 'process.sh calls getStatus.py and getStatus2.py'

grep -v "SUB" tasks > tasks.thin
grep "R" tasks.thin > tasks.R
grep "N" tasks.thin > tasks.N

rm temp
touch temp

for i in {0..9}
do
  ./getStatus.py CMstatus000$i CMs000$i $step
  ./getStatus2.py CMs000$i temp temp2
  mv temp2 temp
done

for i in {10..99}
do
  ./getStatus.py CMstatus00$i CMs00$i $step
  ./getStatus2.py CMs00$i temp temp2
  mv temp2 temp
done

echo "Processed 100 ..."

for i in {100..255}
do
  ./getStatus.py CMstatus0$i CMs0$i $step
  ./getStatus2.py CMs0$i temp temp2
  mv temp2 temp
  if [ "$i" -eq "200" ]
  then
    echo "Processed 200 ..."
  fi
done

mv temp power
./getCost.py 1000 5400 power taskcost $step 0.8 256
