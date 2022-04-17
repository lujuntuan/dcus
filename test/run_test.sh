#!/bin/sh

DCUS_PART_LIBDIR=
DCUS_PART_BINDIR=
DCUS_NAME_PREFIX="domain"
DCUS_TEST_COUNT=10

usage_func() {
    echo "Usage:"
    echo -e "run_test.sh \n \
    [-l DCUS_PART_LIBDIR] \n \
    [-b DCUS_PART_BINDIR] \n \
    [-n DCUS_NAME_PREFIX] \n \
    [-c DCUS_TEST_COUNT]"
    exit -1
}

while getopts 'l:b:n:c:h' sd_setup_flag
do
    case $sd_setup_flag in
        l) DCUS_PART_LIBDIR="$OPTARG";;
        b) DCUS_PART_BINDIR="$OPTARG";;
        n) DCUS_NAME_PREFIX="$OPTARG";;
        c) DCUS_TEST_COUNT="$OPTARG";;
        h) usage_func;;
        ?) usage_func;;
    esac
done

echo "Run DCus test..."
export LD_LIBRARY_PATH=$DCUS_PART_LIBDIR:$LD_LIBRARY_PATH
for i in $(seq 1 $DCUS_TEST_COUNT)
do
    echo "Start ${DCUS_NAME_PREFIX}_${i}..."
    ${DCUS_PART_BINDIR}/dcus_client_simple --name=${DCUS_NAME_PREFIX}_${i} &
    usleep 100000
done

read -p "Kill all?  [y/n]" USER_KILL

if [ "${USER_KILL}" = "n" -o "${USER_KILL}" = "N" ]; then
    echo "Exit."
    exit 0
fi

killall -9 dcus_client_simple

echo "Done."

exit 0
