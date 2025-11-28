#!/bin/bash

config="$1"
appName="CharmApp"

CopyAssets () {
    mode="${1^}"
    system="$2"
    cp -r "$appName/assets/" "bin/$mode-$system/$appName/"
}

RunApplication () {
    cd "bin/$1-$2/$appName"
    LD_LIBRARY_PATH="../Charm/" ./$appName
    cp "imgui.ini" "../../../"
}

CleanProject () {
	rm -rf bin build
	rm Makefile
    rm Charm/Makefile
	rm CharmApp/Makefile
}

BuildProject () {
	premake5 export-compile-commands
	cp "compile_commands/debug.json" "compile_commands.json"
	premake5 gmake
	make all config=$config -j7
    CopyAssets $1 $2
}

if [[ $config = "run" ]]; then
    RunApplication ${2^} $3
elif [[ $config = "clean" ]]; then
    CleanProject
elif [[ $config = "assets" ]]; then
    CopyAssets $2 $3
else
    mode="${config^}"
    BuildProject $mode $2
fi
