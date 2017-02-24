#!/bin/bash

while getopts "a:c:i:r:d:" opt ; do
    case $opt in
        a)
            make -f $OPTARG -j8 DEBUG=NO
            make -f $OPTARG -j8 DEBUG=YES
            ;;
        i)
            make -f $OPTARG -j8 DEBUG=NO
            if [ ! -n $ENV_PATH ]; then
                sudo make -f $OPTARG -j8 install DEBUG=NO
            else
                make -f $OPTARG -j8 install DEBUG=NO ENV_PATH=$ENV_PATH
            fi

            make -f $OPTARG -j8 DEBUG=YES
            if [ ! -n $ENV_PATH ]; then
                sudo make -f $OPTARG -j8 install DEBUG=YES
            else
                make -f $OPTARG -j8 install DEBUG=YES ENV_PATH=$ENV_PATH
            fi
            ;;
        c)
            make -f $OPTARG clean
            ;;
        r)
            make -f $OPTARG -j8 DEBUG=NO
            ;;
        d)
            make -f $OPTARG -j8 DEBUG=YES
            ;;
    esac
done

