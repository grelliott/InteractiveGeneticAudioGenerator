#! /bin/bash

AUDIOGEN_INSTALL_PREFIX=./_install
AUDIOGEN_CONFIG_FILE=foo.yaml
SUPERCOLLIDER_PATH=SuperCollider
SUPERCOLLIDER_SCRIPT=serverconfig.scd

export DISPLAY=:0
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

# start jackd
#/usr/local/bin/jackd -ndefault -P75 -p512 -dalsa -dhw:sndrpihifiberry -r48000 -n2 -D -Phw:sndrpihifiberry,0 -i2 &

# start SuperCollider
echo " " | sclang -D $SUPERCOLLIDER_PATH/$SUPERCOLLIDER_SCRIPT & echo $! > /tmp/sclang.pid

sleep 10

# start audiogen
$AUDIOGEN_INSTALL_PREFIX/bin/audiogen --config $AUDIOGEN_CONFIG_FILE

killall sclang
killall scsynth
