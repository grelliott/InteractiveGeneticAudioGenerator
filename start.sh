#! /bin/bash

# include script for tuning performance
# do during startup
# start jackd (do this at startup)
/usr/local/bin/jackd -R -P90 -dalsa -dhw:sndrpihifiberry -r48000 -Phw:sndrpihifiberry,0 -o2 -i0 -p128 &

BASE_PATH=$PWD
AUDIOGENE_PATH=$BASE_PATH/gene
AUDIOGENE_INSTALL_PATH=$AUDIOGENE_PATH/_install
AUDIOGENE_CONFIG_FILE=$AUDIOGENE_PATH/config.yaml
AUDIOGENE=$AUDIOGENE_INSTALL_PATH/local/bin/audiogene

SUPERCOLLIDER_PATH=$BASE_PATH/audio
SUPERCOLLIDER_SCRIPT=$SUPERCOLLIDER_PATH/serverconfig.scd
SCLANG=sclang

LOGPATH=/var/log/audiogene

export DISPLAY=:0
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

# start SuperCollider
#echo " " | $SCLANG -D $SUPERCOLLIDER_SCRIPT & echo $! > /tmp/sclang.pid

$SCLANG -D $SUPERCOLLIDER_SCRIPT >$LOGPATH/sclang-${BASHPID}.log 2>&1 &

sleep 10

# start audiogen
$AUDIOGENE --config $AUDIOGENE_CONFIG_FILE --log $LOGPATH/audiogene-${BASHPID}.log &

