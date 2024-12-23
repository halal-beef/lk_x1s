#!/bin/bash

case $1 in
	maestro8895)
		rm -rf build-$1; make $1 $2 -j16
		;;
	*)
		echo "-----------------------------------------------------------------"
		echo % usage : ./build.sh [board name] [none / user]
		echo % user mode is not entering ramdump mode when problem happened.
		echo % board list
		echo maestro9610
		echo universal9630
		echo maestro9820
		echo smdk9830
		echo universal9830_bringup
		echo phoenix9830
		echo erd3830
		echo universal3830
		echo c1s
		echo x1s
		echo "-----------------------------------------------------------------"
		exit 0
		;;
esac
