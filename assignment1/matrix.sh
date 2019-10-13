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
        column="$(cat $1 | cut -f $i)"
        echo $column | tr -s ' ' '\t' >> $tresult
        
    done
    
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

mean()
{

    rows="$(dims $1 | cut -d ' ' -f 1)"

    mat="_mtemp"
    cat $1 > $mat 
    transpose $mat
    mresult="_mean$$"
    while read -a row; do
        total=0
        for i in "${row[@]}"; do
            let total+=$i
        done
        printf '%s\t' "$(( ($total + ($rows/2)*( ($total>0)*2-1) ) / $rows ))" >> $mresult
    done < $tresult

    printf '%s\n' "`cat $mresult | sed 's/^[ \t]*//;s/[ \t]*$//'`"
    return 1
}

multiply()
{

    multresult="_multresult$$"
    mat2="_multtemp$$"

    cat $2 > $mat2
    transpose $mat2
    while read -a mat1row; do
        
        while read -a mat2row; do
            dot_product=0
            for i in ${!mat1row[@]}; do
                let dot_product+=${mat1row[$i]}*${mat2row[$i]}
            done
            printf "%s\t" "$dot_product" >> $multresult
        done < $tresult

        printf "\n" >> $multresult

    done < $1

    printf '%s\n' "`cat $multresult | sed 's/^[ \t]*//;s/[ \t]*$//'`"

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
    cat $tresult
    
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

elif [ "$1" == "mean" ]; then

    mat="_input_matrix$$"
     if  [ $# -eq 1 ]; then
        cat > $mat
    elif [ $# -eq 2 ]; then
        mat=$2
    else
        echo "invalid arguments" 1>&2
        exit 1
    fi

    mean $mat

elif [ "$1" == "multiply" ]; then

    if (( $# < 3 )); then
        echo "invalid arguments" 1>&2
        exit 1
    fi

    cols1="$(dims $2 | cut -d ' ' -f 2)"
    rows2="$(dims $3 | cut -d ' ' -f 1)"


    if [[ "$cols1" != "$rows2" ]]; then
        echo "mismatched matricies" 1>&2
        exit 1
    fi

    multiply $2 $3

else
    echo "invalid operator" 1>&2
    exit 1
fi


rm -f _*
exit 0