#!/bin/bash

while getopts "a:c:i:r:d:" opt ; do
    case $opt in
        a)
            make -f $OPTARG -j8 DEBUG=NO
            make -f $OPTARG -j8 DEBUG=YES
            ;;
        i)
            make -f $OPTARG -j8 DEBUG=NO
            sudo make -f $OPTARG -j8 install DEBUG=NO
            make -f $OPTARG -j8 DEBUG=YES
            sudo make -f $OPTARG -j8 install DEBUG=YES
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

