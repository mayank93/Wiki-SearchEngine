#include <cstdio>
#include <cmath>
#include <cstring>
#include <ctime>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include<map>
#include<set>
#include<iostream>
#include <fstream>
#include <sstream>
#include "porter2_stemmer.h"

using namespace std;

#define MAX_TERMS 15
#define MAX_DOC_NUM 300000
#define MULTI_MATCH_BOOST 2.0
#define MIN_WORD_COUNT 1 // To prevent short document normalization issues
#define _FILE_OFFSET_BITS 64
#define ulli unsigned long long int
ulli doccnt=15000000; /* total number of doc */
ulli docWordCount=1; /* get the number of word in doc */
ulli countField[7]={1,1,1,1,1,1,6};
char nameField[6][2]={"B","C","I","O","R","T"};

vector<pair<string,off_t> > vocabDict; /* store secondary index of vocabulary */
vector<pair<ulli,off_t > > documentDict; /* store secondary index of documnetInfo */

vector < pair < double, ulli > > finalRanklist;

map<string,ulli> stopWord;

ifstream postingFile;
ifstream vocabularyFile;
ifstream docFile;

bool my_function (string i,string j){ 
    return (strtoul(i.c_str(),NULL, 0) < strtoul(j.c_str(),NULL, 0));
}

bool my_func(pair < string, off_t > a,pair < string, off_t > b){
    int x = strtoul(a.first.c_str() ,NULL, 0); 
    int y = strtoul(b.first.c_str() ,NULL, 0); 
    return x < y;
}


double get_time(const timespec &a){
    return a.tv_sec+double(a.tv_nsec)/1e9;
}

bool stop_word(string t){
        if(stopWord.find(t)!= stopWord.end())
                    return true;
            return false;
}

vector<string> splitString(string s, char d) {
    ulli l = s.length();
    string token = "";
    vector <string> v;
    for(ulli i = 0; i < l; i++){
        if(s[i] == d) {
            v.push_back(token);
            token = "";
        }
        else token += s[i];
    }
    if(token.size() > 0) v.push_back(token);
    return v;
}


ulli get_doc_wc(ulli docID) {
    pair<ulli,off_t> p;
    p.first = docID;
    p.second=0;
    auto it = lower_bound(documentDict.begin(),documentDict.end(), p);
    //cout<<"------888-"<<endl;
    //cout<<it->first<<" "<<it->second<<endl;
    //cout<<"----88---"<<endl;
    off_t lid;
    if(it == documentDict.end()){ 
        lid = documentDict.size()-1;
    }else{
        lid = it - documentDict.begin();
    }
    if ( documentDict[lid].first != docID ){
        lid--;
    }
    if(lid<0){
        return MIN_WORD_COUNT;
    }
    /*find wc */

    off_t offset=documentDict[lid].second;
    //cout<<docID<<" "<<"offset wc "<<offset<<" "<<documentDict[lid].first<<endl;

    docFile.seekg (offset, docFile.beg);
    size_t found; 

    string remaining;
    string line;
    while ( getline (docFile,line) ){
        found=line.find(":");
        if( strtoul(line.substr(0,found).c_str(),NULL, 0)==docID ){
            remaining=line.substr(found+1);
            found=remaining.find(":");
            //cout<<"######33333333"<<endl;
            //cout<<remaining.substr(found+1).c_str() <<endl;
            //cout<<"########3"<<endl;
            return strtoul(remaining.substr(found+1).c_str(),NULL, 0);
        }else if( strtoul(line.substr(0,found).c_str(),NULL, 0) > docID ){
            return MIN_WORD_COUNT;
        }
    }
    return MIN_WORD_COUNT;
}

string get_doc_title(ulli docID) {
    pair<ulli,off_t>  p;
    p.first = docID;
    p.second=0;
    auto it = lower_bound(documentDict.begin(),documentDict.end(), p);
    off_t lid;
    if(it == documentDict.end()){ 
        lid = documentDict.size()-1;
    }else{
        lid = it - documentDict.begin();
    }
    if ( documentDict[lid].first != docID ){
        lid--;
    }
    if(lid<0){
        return string(" ");
    }
    /*find title */
    off_t offset=documentDict[lid].second;

    docFile.seekg (offset, docFile.beg);
    size_t found; 

    string remaining;
    string line;
    while ( getline (docFile,line) ){
        found=line.find(":");
        if( strtoul(line.substr(0,found).c_str() ,NULL, 0)==docID ){
            //cout<<line<<endl;
            remaining=line.substr(found+1);
            found=remaining.find(":");
            return remaining.substr(0,found).c_str();
        }else if( strtoul(line.substr(0,found).c_str() ,NULL, 0) > docID ){
            return string(" ");
        }
    }
    return string(" ");
}

void symbol_strip(char *s) {
    char *putp = s;
    while(*s) {
        if(isalnum(*s)) {
            *(putp++) = tolower(*s);
        }
        s++;
    }
   *putp='\0';
}

string reduce_token(char *s) {
    symbol_strip(s);
    string data=string(s);
    if(!stop_word(data)){
        Porter2Stemmer::stem(data);
        //cout<<data<<endl;
        if(!stop_word(data)){
            return data;
        }
    }
    return string("0");
}

vector<pair<ulli,double> > process_token(char *token,ulli mod) {
    vector<pair<ulli,double> > ret;
    string reduceToken = reduce_token(token);

    if(reduceToken.compare("0") != 0){
        map<ulli,ulli> filteredFreq;
        pair<string,off_t> p;
        p.first = reduceToken;
        p.second = 0;

        /* find the offset for secondary vocab_list */

        //cout<<"mayank"<<endl;
        auto pt = lower_bound(vocabDict.begin(),vocabDict.end(),p);
        //cout<<"mayank"<<endl;
        off_t lid;
        if(pt == vocabDict.end()){ 
            lid = vocabDict.size()-1;
        }else{
            lid = pt - vocabDict.begin();
        }
        //cout<<"mayank"<<endl;
        if ( vocabDict[lid].first.compare(reduceToken)!=0){
            lid--;
        }
        if(lid<0){
            return ret;
        }

       // cout<<"mayank"<<endl;
        //cout << vocabDict[lid].first <<" "<< vocabDict[lid].second <<endl;
        off_t offset=vocabDict[lid].second;
        vocabularyFile.seekg (offset, vocabularyFile.beg);
        size_t found; 
        off_t offsetArr[countField[mod]];

        /* find the pointer to all postinglist if mod=-1 else specific list */
        reduceToken+="-";
        if(mod!=6){
            reduceToken+=string(nameField[mod]);
        }
        //cout<<"mayank"<<endl;
        //cout<<"maaynk reduce token"<<reduceToken<<endl;
        ulli ind=0;
        string remaining;
        string line;
        while ( getline (vocabularyFile,line) ){
            //cout<<"-----"<<endl;
            //cout<<line<<endl;
            //cout<<"-----"<<endl;
            if(line.compare(0, reduceToken.size(), reduceToken)==0){
                //cout<<"---****--"<<endl;
                //cout<<line<<endl;
                //cout<<"--****---"<<endl;
                found=line.find(":");
                int weight=1;
                char a=line[found-1];
                //cout<<a<<endl;
                if(a=='T'){
                 //   printf("mayank\n");
                    weight=1;
                }else if(a!='B'){
                    weight=1;
                }
                remaining=line.substr(found+1);
                found=remaining.find(":");
                
                offsetArr[ind++]=strtoul(remaining.substr(0,found).c_str() ,NULL, 0);
                
                postingFile.seekg (offsetArr[ind-1], postingFile.beg);
                getline (postingFile,line);
                vector <string> tokens = splitString(line, ',');
                ulli tokenLen = tokens.size();
                // go to each point in poistinglist for a given pointer and get all docID-tf pair in to map filteredFreq 
                for(ulli j = 0; j < tokenLen; j++) {
                    if(j>MAX_DOC_NUM){
                        break;
                    }
                    vector <string> freq(2);
                    freq = splitString(tokens[j], '-');
                    //cout << freq[0] << " " << strtoul(freq[1].c_str(),NULL, 0) << endl;
                    ulli key, val;
                    key = strtoul(freq[0].c_str(),NULL, 0);
                    val = strtoul(freq[1].c_str(),NULL, 0);
                    //cout<<"------"<<endl;
                    //cout<<key<<" "<<val<<endl;
                    //cout<<"------"<<endl;
                    //cout << key << " " << val << endl;
                    if(filteredFreq.find(key) == filteredFreq.end()){
                        filteredFreq[key] = val*weight;
                    }else{ 
                        filteredFreq[key] += val*weight;
                    }
                 //   if(key==119886){
                   //     cout<<key<<" "<<val<<" "<<weight<<endl;
                     //   cout<<filteredFreq[key]<<endl;
                   // }
                }
                //               cout<<"--**---"<<endl;
                //               cout<<offsetArr[ind-1]<<endl;
                //               cout<<"--**---"<<endl;
                if(ind>=countField[mod]){
                    break;
                }
            }else if( line.compare(0, reduceToken.size(), reduceToken)>0 ){
                break;              
            }
        }

        ulli elcnt = filteredFreq.size();
        cout<<elcnt<<endl;
        double idf = log(doccnt/double(elcnt));
        //cout<<idf<<endl;
        for(auto it : filteredFreq) { 
            pair<ulli,double> p;
            p.first = it.first;
            //cout<<it.first<<" "<<it.second<<endl;
            p.second = idf*double(it.second)/get_doc_wc(p.first);
            //cout<<p.second<<endl;
            //cout<<p.first<<" "<<p.second<<endl;
            ret.push_back(p);
        }
    }
    //cout<<"--**---"<<endl;
    //cout<<ret.size() <<endl;
    //cout<<"--**---"<<endl;
    return ret;
}



vector<pair<string,ulli> > process_input() {
    char buf[10240];
    char buf2[10240];
    vector<pair<string,ulli> > ret;

    if(scanf(" %[^\n]",buf)==EOF) {
        ret.push_back(make_pair("EOF",-2));
    }
    
    ulli putp=0;
    char *p = buf;
    while(*p) {
        if(*p==':') {
            buf2[putp++] = ' ';
            buf2[putp++] = *p;
            buf2[putp++] = ' ';
        }else if( *p<'A' || *p>'z' || ( *p>'Z' && *p<'a' ) ){
            buf2[putp++] = ' ';
        } else {
            buf2[putp++] = *p;
        }
        p++;
    }
    buf2[putp] = 0;

    vector<string> toks;
    for(char* tok = strtok(buf2," \t"); tok != NULL; tok = strtok(NULL," \t")) {
        toks.push_back(string(tok));
    }
    int mode = 6;
    for(size_t i = 0; i < toks.size(); i++) {
        bool modifier = false;
        if(i+1 < toks.size()) {
            if(toks[i+1] == ":") {
                modifier = true;
                if(toks[i] == "T")
                    mode = 0;
                else if(toks[i] == "C")
                    mode = 1;
                else if(toks[i] == "B")
                    mode = 2;
                else if(toks[i] == "I")
                    mode = 3;
                else if(toks[i] == "O")
                    mode = 4;
                else if(toks[i] == "R")
                    mode = 5;
                else {
                    modifier = false;
                }
            }
        }
        if(modifier)
            i++;
        else
            ret.push_back(make_pair(toks[i],mode));
    }
    return ret;
}

int main(int argc,char*argv[]){
    string line;
    string word;
    string var;
    string ans="";
    vector<string> pair;
    string indexPath=string(argv[1]);
    string indexFilePath=indexPath+"/index.txt";
    
    postingFile.open(indexPath+"/posting.txt");
    if (!postingFile.is_open()){
        cout<<"Error opening posting.txt"<<endl;
        exit(0);
    }

    vocabularyFile.open(indexPath+"/vocabulary.txt");
    if (!vocabularyFile.is_open()){
        cout<<"Error opening vocabulary.txt"<<endl;
        exit(0);
    }

    docFile.open(indexPath+"/sortedDocTitle.txt");
    if (!docFile.is_open()){
        cout<<"Error opening sortedDocTitle.txt"<<endl;
        exit(0);
    }

    ifstream myfile(argv[2]);
    if (myfile.is_open()){
        while ( getline (myfile,line) ){
            stopWord[line]=1;
        }
        myfile.close();
    }else{
        cout<<"Error opening stopwords.txt"<<endl;
        exit(0);
    }
    printf("loading stopword done\n");
    
    size_t found;
    
    /*load vocabuary secondaryindex in memory*/
    ifstream myfile1(indexPath+"/secondaryVocab.txt");

    if (myfile1.is_open()){
        while ( getline (myfile1,line) ){
            found=line.find(":");
            vocabDict.push_back(make_pair(line.substr(0,found),strtoul(line.substr(found+1).c_str(),NULL, 0)));
            //vocabDict[line.substr(0,found)]=strtoul(line.substr(found+1).c_str(),NULL, 0);
        }
        myfile1.close();
    }else{
        cout<<"Error opening secondaryVocab.txt"<<endl;
        exit(0);
    }
//        cout<<vocabDict["agama"]<<endl;
    printf("loading secondaryvocab  done\n");

    /*load docID,title secondaryindex in memory*/
    ifstream myfile2(indexPath+"/secondaryDoc.txt");
 //   int ccc=0;
    if (myfile2.is_open()){
        while ( getline (myfile2,line) ){
            found=line.find(":");
            documentDict.push_back(make_pair(strtoul(line.substr(0,found).c_str(),NULL, 0),strtoul(line.substr(found+1).c_str(),NULL, 0)));
   //         ccc++;
  //         if(ccc<10)
    //        cout<<strtoul(line.substr(0,found).c_str(),NULL, 0)<<" "<<strtoul(line.substr(found+1).c_str(),NULL, 0)<<endl;
          //  documentDict[strtoul(line.substr(0,found).c_str(),NULL, 0)]=strtoul(line.substr(found+1).c_str(),NULL, 0);
        }
        myfile2.close();
    }else{
        cout<<"Error opening secondaryDoc.txt"<<endl;
        exit(0);
    }
    
    printf("loading  secondaryDoc done\n");
    /*get doc count and store in doccnt */
//    cout<<get_doc_title((ulli)595)<<endl;
    while (1){
        finalRanklist.clear();
        printf("Enter query :\n");
        auto tokens = process_input();
  //      printf("sdjgsddfjhdvf\n");
        timespec start_time,end_time;
        clock_gettime(CLOCK_MONOTONIC,&start_time);

        if(tokens.size()>0 && tokens[0].first.compare("EOF") == 0)
            break;
    //    printf("sdjgsddfjhdvf\n");

        unordered_map<ulli,double> ranklist;
        int flag=0;
        for(auto token : tokens) {
            char buf3[100];
            strcpy(buf3,token.first.c_str());
            auto tfidf = process_token(buf3,token.second);
            for(auto it: tfidf){
                flag=1;
                if(ranklist.find(it.first)==ranklist.end()){
                    ranklist[it.first] = MULTI_MATCH_BOOST+it.second; // constant extra score  for boosting multiple term match
                }else{
                    ranklist[it.first] += MULTI_MATCH_BOOST+it.second; // constant extra score  for boosting multiple term match
                }
            }
        }
        if(flag==1){

            for (auto it : ranklist){
                finalRanklist.push_back(make_pair(it.second,it.first));
            }
            sort(finalRanklist.begin(),finalRanklist.end());

            clock_gettime(CLOCK_MONOTONIC,&end_time);
            printf("Query took %lf seconds\n",get_time(end_time) - get_time(start_time));

            int c= 0;
            for(ulli i = finalRanklist.size() - 1; i >=0 && c <  MAX_TERMS ; i--,c++) {
                printf("%d. %llu %s\n",c+1,finalRanklist[i].second,get_doc_title(finalRanklist[i].second).c_str());
            }
        }else{
            printf("NOT FOUND\n");
        }
    }
    return 0;
}
