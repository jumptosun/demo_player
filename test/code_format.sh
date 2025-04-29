#!/bin/bash 

# 使用 clang-format 格式化代码

file_to_format=""

get_format_file() {
    file_to_format="${file_to_format} $(find . -name "*.h")"
    file_to_format="${file_to_format} $(find . -name "*.hpp")"

    file_to_format="${file_to_format} $(find . -name "*.c")"
    file_to_format="${file_to_format} $(find . -name "*.cc")"
    file_to_format="${file_to_format} $(find . -name "*.cpp")"
}

format_folder() {
    get_format_file

    for file in ${file_to_format}; do
        echo "clang-format -i ${file} -style WebKit"
        clang-format -i ${file} -style WebKit
    done
}

main() {
    dir_to_format=""

    while getopts 'd:' OPT; do
        case $OPT in
        d)
            dir_to_format=${OPTARG}
            ;;
        ?)
            echo "usage: code_format.sh -d dir_to_format"
            exit 0
            ;;
        esac
    done

    if [ ! "${dir_to_format}" ] ; then
        echo "usage: code_format.sh -d dir_to_format"
        exit 0
    else
        echo "format dir: ${dir_to_format}"

        cd ${dir_to_format}
        format_folder 
    fi
}

main $*

