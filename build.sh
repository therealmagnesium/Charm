#!/bin/bash

config="$1"
appName="CharmApp"

if [[ $config = "run" ]]; then
	mode="$2"
	system="$3"

	if [[ $mode = "debug" ]]; then
		cd "bin/Debug-$system/$appName"
		LD_LIBRARY_PATH="." ./$appName
	elif [[ $mode = "release" ]]; then
		cd "bin/Release-$system/$appName"
		LD_LIBRARY_PATH="." ./$appName
	elif [[ $mode = "dist" ]]; then
		cd "bin/Dist-$system/$appName"
		LD_LIBRARY_PATH="." ./$appName
	fi
elif [[ $config = "assets" ]]; then
	mode="$2"
	system="$3"
	if [[ $mode = "debug" ]]; then
		cp -r "$appName/assets/" "bin/Debug-$system/$appName/"
	elif [[ $mode = "release" ]]; then
		cp -r "$appName/assets/" "bin/Release-$system/$appName/"
	elif [[ $mode = "dist" ]]; then
		cp -r "$appName/assets/" "bin/Dist-$system/$appName/"
	fi
elif [[ $config = "clean" ]]; then
	rm -rf bin build
	rm Makefile
    rm Charm/Makefile
	rm CharmApp/Makefile
else
	system="$2"
	premake5 export-compile-commands
	cp "compile_commands/debug.json" "compile_commands.json"
	premake5 gmake
	make all config=$config -j7
fi
