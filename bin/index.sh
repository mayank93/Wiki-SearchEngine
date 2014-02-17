#!/bin/bash
./parse $1 $2 ../src/index/stopwords.txt
./merging.sh $2
