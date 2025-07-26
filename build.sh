#!/bin/bash

config="$1"
appName="CharmApp"

CopyAssets () {
    mode="${config^}"
    system="$2"
    cp -r "$appName/assets/" "bin/$mode-$system/$appName/"
}

CopySandboxProject () {
    mode="${config^}"
    system="$2"
    cp -r "$appName/SandboxProject/" "bin/$mode-$system/$appName"
}

RunApplication () {
    cd "bin/$1-$2/$appName"
    LD_LIBRARY_PATH="." ./$appName
}

CleanProject () {
	rm -rf bin build
	rm Makefile
    rm Charm/Makefile
	rm CharmApp/Makefile
    rm -rf CharmApp/SandboxProject/assets/scripts/binaries
    rm -rf CharmApp/SandboxProject/assets/scripts/intermediates
    rm CharmApp/SandboxProject/Makefile
    rm CharmApp/SandboxProject/*.make
}

BuildProject () {
	premake5 export-compile-commands
	cp "compile_commands/debug.json" "compile_commands.json"
	premake5 gmake
	make all config=$config -j7
    cd "$appName/SandboxProject"
    premake5 export-compile-commands
	cp "compile_commands/debug.json" "compile_commands.json"
    premake5 gmake
	make all config=$config -j7
    cd "../.."
    CopyAssets $1 $2
    CopySandboxProject $1 $2
}

if [[ $config = "run" ]]; then
    RunApplication ${2^} $3
elif [[ $config = "clean" ]]; then
    CleanProject
else
    mode="${config^}"
    BuildProject $mode $2
fi
