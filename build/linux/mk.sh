#!/bin/bash

while getopts "a:r:d:" opt ; do
    case $opt in
        a)
            make -f $OPTARG -j8 DEBUG=NO
            make -f $OPTARG -j8 DEBUG=YES
            ;;
        r)
            make -f $OPTARG -j8 DEBUG=NO
            ;;
        d)
            make -f $OPTARG -j8 DEBUG=YES
            ;;
    esac
done

