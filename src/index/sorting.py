#!/bin/python
import sys
indexfolder=sys.argv[1]
indexfile=open(indexfolder+'/'+sys.argv[2],'r')
vocabulary=open(indexfolder+'/'+'vocabulary.txt','w')
posting=open(indexfolder+'/'+'posting.txt','w')

for i in indexfile:
        if i:
            position = posting.tell();
            i=i.split(':')
            word=i[0]
            postinglist=i[1].strip('\n\r,').split(',')
            postinglist.sort(key=lambda x: x.split('-')[1],reverse=True)
            posting.write(','.join(postinglist)+'\n')
            postinglen=posting.tell()-position;
            vocabulary.write(word+':'+str(position)+':'+str(postinglen)+'\n')
            position=position+postinglen
indexfile.close()
posting.close()
vocabulary.close()
