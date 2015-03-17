#!/bin/bash

while getopts "a:i:r:d:" opt ; do
    case $opt in
        a)
            make -f $OPTARG -j8 DEBUG=NO
            make -f $OPTARG -j8 DEBUG=YES
            ;;
        i)
            make -f $OPTARG -j8 DEBUG=NO
            make -f $OPTARG -j8 DEBUG=YES
            sudo make -f $OPTARG install DEBUG=NO
            sudo make -f $OPTARG install DEBUG=YES
            ;;
        r)
            make -f $OPTARG -j8 DEBUG=NO
            ;;
        d)
            make -f $OPTARG -j8 DEBUG=YES
            ;;
    esac
done

