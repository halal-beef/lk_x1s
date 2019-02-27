#!/bin/bash

case $1 in
	maestro9610)
		rm -rf build-$1; make $1 -j16
		;;
	maestro9820)
		rm -rf build-$1; make $1 -j16
		;;
	smdk9830)
		rm -rf build-$1; make $1 -j16
		;;
	*)
		echo "-----------------------------------------------------------------"
		echo % usage : ./build.sh [board name]
		echo % board list
		echo maestro9610
		echo maestro9820
		echo smdk9830
		echo "-----------------------------------------------------------------"
		exit 0
		;;
esac
