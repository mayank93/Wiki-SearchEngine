#!/bin/bash
import sys
indexfolder=sys.argv[1]
docfile=open(indexfolder+'/'+sys.argv[2],'r')
secondaryfile=open(indexfolder+'/'+'secondaryDoc.txt','w')
count=99;

while 1:
    i = docfile.readline()
    if not i:
        break
    count+=1
    if count==100:
        position = docfile.tell()-len(i)
        secondaryfile.write(i.split(':')[0]+':'+str(position)+'\n')
        count=0;

docfile.close()
secondaryfile.close()
