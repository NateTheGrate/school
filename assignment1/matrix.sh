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
    tresult="_transpose_result$$"
    cols="$(dims $1 | cut -d ' ' -f 2)"

    for (( i=1; i<=$cols; i++ )); do
        
        column="$(cat $mat | cut -f $i)"
        echo $column | tr -s ' ' '\t' >> $tresult
        
    done

    cat $tresult
    return 0
}

add()
{
    aresult="_addition$$"
    cols="$(dims $2 | cut -d ' ' -f 2)"
    rows="$(dims $2 | cut -d ' ' -f 1)"
    
    arr1=()
    while read -a row; do
        for i in "${row[@]}"; do
            
            arr1+=( $i )

        done
    done < $1
    
    arr2=()
    while read -a row; do
        for i in "${row[@]}"; do
            
            arr2+=( $i )

        done
    done < $2
    
    j=0
    for (( i=0; i<$rows*$cols; i++ )); do

        if (( ($i+1)%$cols == 0 )); then 
            printf "%d\n" `expr ${arr1[i]} + ${arr2[i]}` >> $aresult
        else
            printf "%d\t" `expr ${arr1[i]} + ${arr2[i]}` >> $aresult
        fi
    done
    
    cat $aresult
    return 0

}

## start of main script ##

if [ "$1" == "dims" ]; then
    mat="_input_matrix$$"
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
    mat="_input_matrix$$"
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

    transpose $mat
    
elif [ "$1" == "add" ]; then
    
    if (( $# < 3 )); then

        echo "invalid arguments" 1>&2
        exit 1

    fi

    cols1="$(dims $2 | cut -d ' ' -f 2)"
    rows1="$(dims $2 | cut -d ' ' -f 1)"

    cols2="$(dims $3 | cut -d ' ' -f 2)"
    rows2="$(dims $3 | cut -d ' ' -f 1)"


    if [[ "$cols2" != "$cols1" || "$rows1" != "$rows2" ]]; then
        echo "mismatched matricies" 1>&2
        exit 1
    fi
    add $2 $3

else
    echo "invalid operator" 1>&2
    exit 1
fi


rm -f _*
exit 0