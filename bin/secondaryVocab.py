#!/bin/bash
import sys
indexfolder=sys.argv[1]
docfile=open(indexfolder+'/'+sys.argv[2],'r')
secondaryfile=open(indexfolder+'/'+'secondaryVocab.txt','w')
count=99;

prevWord=""
while 1:
    i = docfile.readline()
    if not i:
        break
    count+=1
    if count>=100 and prevWord!=(i.split('-')[0]):
        position = docfile.tell()-len(i)
        secondaryfile.write(i.split('-')[0]+':'+str(position)+'\n')
        count=0;

docfile.close()
secondaryfile.close()
