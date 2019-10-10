#!/bin/bash
dims()
{
    rowcount=0
    while read row; do
        colcount=0

        ((rowcount++))
        for i in ${row[@]}; do
            
            ((colcount++))
        done
        
    done < $1

    echo "$rowcount $colcount"
    return 0
}


transpose()
{
    result="transpose_result$$"
    cols="$(dims $1 | cut -d ' ' -f 2)"

    for (( i=1; i<=$cols; i++ )); do
        
        column="$(cat $mat | cut -f $i)"
        echo $column | tr -s ' ' '\t' >> $result
        
    done

    cat $result
    return 0


}

## start of main script

if [ "$1" == "dims" ]; then
    mat=0
    if  [ $# -eq 1 ]; then
        cat > $mat
    elif [ $# -eq 2 ]; then
        mat=$2
    else
        echo "invalid arguments" 1>&2
        exit 1
    fi

    if  [ ! -r $mat ]; then
        echo "a file is not readable" 1>&2
        exit 1
    fi

    dims $mat

elif [ "$1" == "transpose" ]; then
    mat=0
    if  [ $# -lt 2 ]; then
        cat > $mat
    else
        mat=$2
    fi
    if  [ ! -r $mat ]; then
        echo "a file is not readable" 1>&2
        exit 1
    fi


    transpose $2

else
    echo "invalid operator" 1>&2
    exit 1
fi