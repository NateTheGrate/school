#!/bin/bash

# author: Nathanael Butler
# date: 10/13/2019
# Computes the dimensions, transpose, sum, mean, and product of matricies.


# function to find the dimensions of a matrix
# takes a variable that contains an MxN matrix
# outputs dimensions via stdout
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

# function to find transpose of a matrix
# takes a file that contains an MxN matrix
# outputs transpose via stdout
transpose()
{
    # intialize temp file
    tresult="_transpose_result$$"
    
    # loop through every column
    cols="$(dims $1 | cut -d ' ' -f 2)"
    for (( i=1; i<=$cols; i++ )); do

        # cut input by every tab, then replace spaces with tabs
        column="$(cat $1 | cut -f $i)"
        echo $column | tr -s ' ' '\t' >> $tresult
        
    done
    
    return 0
}

# function to add 2 matricies to eachother
# takes two variables that each contain an MxN matrix of the same size
# outputs sum via stdout
add()
{
    # intialize temp file
    aresult="_addition$$"

    # need dimensions for later
    cols="$(dims $2 | cut -d ' ' -f 2)"
    rows="$(dims $2 | cut -d ' ' -f 1)"
    
    # put matrix into 1d array
    arr1=()
    while read -a row; do
        for i in "${row[@]}"; do
            
            arr1+=( $i )

        done
    done < $1
    
    # put matrix into 1d array
    arr2=()
    while read -a row; do
        for i in "${row[@]}"; do
            
            arr2+=( $i )

        done
    done < $2
    
    # loop through every element of matrix (length * height)
    for (( i=0; i<$rows*$cols; i++ )); do

        # if next index is at the end of a row, print new line
        if (( ($i+1)%$cols == 0 )); then 
            printf "%d\n" `expr ${arr1[i]} + ${arr2[i]}` >> $aresult
        # otherwise, put a tab at the end
        else
            printf "%d\t" `expr ${arr1[i]} + ${arr2[i]}` >> $aresult
        fi
    done
    
    # output result
    cat $aresult
    return 0

}

# function to find the average of each row
# takes one variable that contains an MxN matrix
# outputs a 1xN matrix with the average of each column as each entry
mean()
{
    # intialize temp files
    mresult="_mean$$"
    mat="_mtemp"

    # using transpose to make it easier to read
    cat $1 > $mat 
    transpose $mat

    # used as a divisor later
    rows="$(dims $1 | cut -d ' ' -f 1)"

    # sum each line
    while read -a row; do
        total=0
        for i in "${row[@]}"; do
            let total+=$i
        done

        # round average and append to output
        printf '%s\t' "$(( ($total + ($rows/2)*( ($total>0)*2-1) ) / $rows ))" >> $mresult
    done < $tresult

    # trim tabs and print output
    printf '%s\n' "`cat $mresult | sed 's/^[ \t]*//;s/[ \t]*$//'`"
    return 1
}

# function to find the product of 2 matricies
# takes 2 variables that contain MxN and NxP matricies respectivley
# outputs an MxP matrix that is the result of multipliying the matricies
multiply()
{
    # initialize temp files
    multresult="_multresult$$"
    mat2="_multtemp$$"

    # using transpose to read the file more easily
    cat $2 > $mat2
    transpose $mat2
    while read -a mat1row; do
        while read -a mat2row; do

            dot_product=0
            for i in ${!mat1row[@]}; do

                # calculate dot product like so:
                # m1[row][i] m2^T[row][i] + m1[row][i+1]m2^T[row][i] ....
                let dot_product+=${mat1row[$i]}*${mat2row[$i]}
            done

            # add result to output
            printf "%s\t" "$dot_product" >> $multresult
        done < $tresult

        # reset on new line
        printf "\n" >> $multresult

    done < $1

    # trim tabs at the end and print output to stdout
    printf '%s\n' "`cat $multresult | sed 's/^[ \t]*//;s/[ \t]*$//'`"
    return 0
}

## start of main script ##

# if first argument isn't dims, transpose, add, mean, multiply... then exit with error
if [ "$1" == "dims" ]; then
    
    mat="_input_matrix$$"
    # if one argument then get input from stdout
    if  [ $# -eq 1 ]; then
        cat > $mat
    # if two arugments then get input from parameters
    elif [ $# -eq 2 ]; then
        mat=$2
    # otherwise, exit with error
    else
        echo "invalid arguments" 1>&2
        exit 1
    fi

    # if file is not readable, then exit with error
    if  [ ! -r $mat ]; then
        echo "a file is not readable" 1>&2
        exit 1
    fi
    # call function
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
    # output result
    cat $tresult
    
elif [ "$1" == "add" ]; then
    
    # if there are not three parameters, then exit with error
    if (( $# != 3 )); then
        echo "invalid arguments" 1>&2
        exit 1
    fi

    # check if inputs are readable
    if  [ ! -r $2 ]; then
        echo "a file is not readable" 1>&2
        exit 1
    fi

    if  [ ! -r $3 ]; then
        echo "a file is not readable" 1>&2
        exit 1
    fi

    # get dimesnions of each matrix
    cols1="$(dims $2 | cut -d ' ' -f 2)"
    rows1="$(dims $2 | cut -d ' ' -f 1)"

    cols2="$(dims $3 | cut -d ' ' -f 2)"
    rows2="$(dims $3 | cut -d ' ' -f 1)"

    # if not same dimensions, then exit with error
    if [[ "$cols2" != "$cols1" || "$rows1" != "$rows2" ]]; then
        echo "mismatched matricies" 1>&2
        exit 1
    fi

    # call function
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

     # if file is not readable, then exit with error
    if  [ ! -r $mat ]; then
        echo "a file is not readable" 1>&2
        exit 1
    fi

    mean $mat

elif [ "$1" == "multiply" ]; then

    if (( $# != 3 )); then
        echo "invalid arguments" 1>&2
        exit 1
    fi

    if  [ ! -r $2 ]; then
        echo "a file is not readable" 1>&2
        exit 1
    fi

    if  [ ! -r $3 ]; then
        echo "a file is not readable" 1>&2
        exit 1
    fi

    # check if matricies are MxN and NxP
    cols1="$(dims $2 | cut -d ' ' -f 2)"
    rows2="$(dims $3 | cut -d ' ' -f 1)"

    if [[ "$cols1" != "$rows2" ]]; then
        echo "mismatched matricies" 1>&2
        exit 1
    fi

    # call function
    multiply $2 $3

else
    echo "invalid operator" 1>&2
    exit 1
fi

# remove all temp files and exit without error
rm -f _*
exit 0