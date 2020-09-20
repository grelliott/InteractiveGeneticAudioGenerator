#! /bin/bash

BASE_PATH=$PWD
AUDIOGENE_PATH=$BASE_PATH/gene
AUDIOGENE_INSTALL_PATH=$AUDIOGENE_PATH/_install
AUDIOGENE_CONFIG_FILE=$AUDIOGENE_PATH/config.yaml
AUDIOGENE=$AUDIOGENE_INSTALL_PATH/local/bin/audiogene

SUPERCOLLIDER_PATH=$BASE_PATH/SuperCollider
SUPERCOLLIDER_SCRIPT=$SUPERCOLLIDER_PATH/serverconfig.scd
SCLANG=sclang

export DISPLAY=:0
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

# start jackd
/usr/local/bin/jackd -ndefault -P75 -p64 -dalsa -dhw:sndrpihifiberry -r48000 -n2 -D -Phw:sndrpihifiberry,0 -i2 &

# start SuperCollider
echo " " | $SCLANG -D $SUPERCOLLIDER_SCRIPT & echo $! > /tmp/sclang.pid
#echo " " | sclang -D $SUPERCOLLIDER_PATH/test.scd& echo $! > /tmp/sclang.pid

sleep 10

# start audiogen
$AUDIOGENE --config $AUDIOGENE_CONFIG_FILE

killall sclang
killall scsynth
