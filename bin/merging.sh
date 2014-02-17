#!/bin/bash
sort -d -t':' -k1 -V $1/index.txt > $1/sorted.txt
rm $1/index.txt

cat $1/sorted.txt | awk  -v FS=':' 'BEGIN{myword=""; nwords=0} { if(myword != $1 ) {printf "\n%s:",$1; myword=$1} printf "%s",$2  } END{printf "\n"}' > $1/merged.txt
rm $1/sorted.txt

A=$(echo "`wc -l $1/merged.txt | cut -d' ' -f1`-1" | bc)

tail -n $A $1/merged.txt > $1/merge.txt
rm $1/merged.txt

python sorting.py $1 merge.txt
rm $1/merge.txt

sort -d -t':' -k1 -V $1/docTitle.txt > $1/sortedDocTitle.txt
rm $1/docTitle.txt

python secondaryDoc.py $1 sortedDocTitle.txt

python secondaryVocab.py $1 vocabulary.txt
