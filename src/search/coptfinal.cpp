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
#define MULTI_MATCH_BOOST 5
#define MIN_WORD_COUNT 1 // To prevent short document normalization issues
#define _FILE_OFFSET_BITS 64

int doccnt=1; /* total number of doc */
int docWordCount=15000000; /* get the number of word in doc */
int countField[7]={1,1,1,1,1,1,6};
char nameField[6][2]={"B","C","I","O","R","T"};

vector<pair<string,off_t> > vocabDict; /* store secondary index of vocabulary */
vector<pair<int,off_t > > documentDict; /* store secondary index of documnetInfo */

vector < pair < double, int > > finalRanklist;

map<string,int> stopWord;

ifstream postingFile;
ifstream vocabularyFile;
ifstream docFile;

bool my_function (string i,string j){ 
    return (atoi(i.c_str()) < atoi(j.c_str()));
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
    int l = s.length();
    string token = "";
    vector <string> v;
    for(int i = 0; i < l; i++){
        if(s[i] == d) {
            v.push_back(token);
            token = "";
        }
        else token += s[i];
    }
    if(token.size() > 0) v.push_back(token);
    return v;
}


int get_doc_wc(int docID) {
    pair<int,off_t> p;
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
        if( atoi(line.substr(0,found).c_str())==docID ){
            remaining=line.substr(found+1);
            found=remaining.find(":");
            //cout<<"######33333333"<<endl;
            //cout<<remaining.substr(found+1).c_str() <<endl;
            //cout<<"########3"<<endl;
            return atoi(remaining.substr(found+1).c_str());
        }else if( atoi(line.substr(0,found).c_str()) > docID ){
            return MIN_WORD_COUNT;
        }
    }
    return MIN_WORD_COUNT;
}

string get_doc_title(int docID) {
    pair<int,off_t>  p;
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
        if( atoi(line.substr(0,found).c_str())==docID ){
            remaining=line.substr(found+1);
            found=remaining.find(":");
            return remaining.substr(0,found).c_str();
        }else if( atoi(line.substr(0,found).c_str()) > docID ){
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
        cout<<data<<endl;
        if(!stop_word(data)){
            return data;
        }
    }
    return string("0");
}

vector<pair<int,double> > process_token(char *token,int mod) {
    vector<pair<int,double> > ret;
    string reduceToken = reduce_token(token);

    if(reduceToken.compare("0") != 0){
        unordered_map<int,int> filteredFreq;
        pair<string,off_t> p;
        p.first = reduceToken;
        p.second = 0;

        /* find the offset for secondary vocab_list */

        auto pt = lower_bound(vocabDict.begin(),vocabDict.end(),p);
        off_t lid;
        if(pt == vocabDict.end()){ 
            lid = vocabDict.size()-1;
        }else{
            lid = pt - vocabDict.begin();
        }
        if ( vocabDict[lid].first.compare(reduceToken)!=0){
            lid--;
        }
        if(lid<0){
            return ret;
        }

        //cout<<"mayank"<<endl;
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
        //cout<<"maaynk reduce token"<<reduceToken<<endl;

        int ind=0;
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
                remaining=line.substr(found+1);
                found=remaining.find(":");

                offsetArr[ind++]=atoi(remaining.substr(0,found).c_str());

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
        /* go to each point in poistinglist for a given pointer and get all docID-tf pair in to map filteredFreq */
        for(int i=0;i<ind;i++){
            postingFile.seekg (offsetArr[i], postingFile.beg);
            //cout<<"--**---"<<endl;
            //cout<<postingFile.tellg() <<endl;
            //cout<<"--**---"<<endl;
            /*read from this point till \n and put it in filteredFreq*/
            getline (postingFile,line);
            //cout<<line<<endl;
            vector <string> tokens = splitString(line, ',');
            int tokenLen = tokens.size();
            for(int j = 0; j < tokenLen; j++) {
                vector <string> freq(2);
                freq = splitString(tokens[j], '-');
                //cout << freq[0] << " " << atoi(freq[1].c_str()) << endl;
                int key, val;
                key = atoi(freq[0].c_str());
                val = atoi(freq[1].c_str());
                filteredFreq[atoi(freq[0].c_str())] = atoi(freq[1].c_str());
                //cout << key << " " << val << endl;
                if(filteredFreq.find(key) == filteredFreq.end()) filteredFreq[key] = val;
                else filteredFreq[key] += val;
            }   
        }

        int elcnt = filteredFreq.size();

        double idf = log(doccnt/double(elcnt));
        //cout<<idf<<endl;
        for(auto it : filteredFreq) { 
            pair<int,double> p;
            p.first = it.first;
            p.second = idf*double(it.second)/get_doc_wc(p.first);
            //cout<<get_doc_wc(p.first)<<endl;
            //cout<<p.first<<" "<<p.second<<endl;
            ret.push_back(p);
        }
    }
            //cout<<"--**---"<<endl;
            //cout<<ret.size() <<endl;
            //cout<<"--**---"<<endl;
    return ret;
}



vector<pair<string,int> > process_input() {
    char buf[10240];
    char buf2[10240];
    vector<pair<string,int> > ret;

    if(scanf(" %[^\n]",buf)==EOF) {
        ret.push_back(make_pair("EOF",-2));
    }
    
    int putp=0;
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
            vocabDict.push_back(make_pair(line.substr(0,found),atoi(line.substr(found+1).c_str())));
            //vocabDict[line.substr(0,found)]=atoi(line.substr(found+1).c_str());
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
//    int ccc=0;
    if (myfile2.is_open()){
        while ( getline (myfile2,line) ){
            found=line.find(":");
            documentDict.push_back(make_pair(atoi(line.substr(0,found).c_str()),atoi(line.substr(found+1).c_str())));
  //          ccc++;
    //        if(ccc<10)
      //      cout<<atoi(line.substr(0,found).c_str())<<" "<<atoi(line.substr(found+1).c_str())<<endl;
           // documentDict[atoi(line.substr(0,found).c_str())]=atoi(line.substr(found+1).c_str());
        }
        myfile2.close();
    }else{
        cout<<"Error opening secondaryDoc.txt"<<endl;
        exit(0);
    }
    
    printf("loading  secondaryDoc done\n");
    /*get doc count and store in doccnt */

    while (1){
        finalRanklist.clear();
        printf("Enter query :\n");
        auto tokens = process_input();

        timespec start_time,end_time;
        clock_gettime(CLOCK_MONOTONIC,&start_time);

        if(tokens.size()>0 && tokens[0].second == -2)
            break;

        unordered_map<int,double> ranklist;
        for(auto token : tokens) {
            char buf3[100];
            strcpy(buf3,token.first.c_str());
            auto tfidf = process_token(buf3,token.second);
            for(auto it: tfidf){
                ranklist[it.first] += MULTI_MATCH_BOOST+it.second; // constant extra score  for boosting multiple term match
            }
        }

        for (auto it : ranklist){
            finalRanklist.push_back(make_pair(it.second,it.first));
        }
        sort(finalRanklist.begin(),finalRanklist.end());

        clock_gettime(CLOCK_MONOTONIC,&end_time);
        printf("Query took %lf seconds\n",get_time(end_time) - get_time(start_time));

        int c= 0;
        for(int i = finalRanklist.size() - 1; i >=0 && c < MAX_TERMS ; i--,c++) {
                 printf("%d.%d %f  %s\n",c+1,finalRanklist[i].second,finalRanklist[i].first ,get_doc_title(finalRanklist[i].second).c_str());
        }
    }
    return 0;
}
