#include <fstream>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <cstring> 
#include <glibmm/convert.h> //For Glib::ConvertError
#include<boost/algorithm/string.hpp>
#include<boost/algorithm/string/trim.hpp>
#include<boost/algorithm/string/predicate.hpp>
#include<boost/algorithm/string/classification.hpp>
#include<map>
#include<set>
#include "myparser.h"
#include "porter2_stemmer.h"

using namespace std;
using namespace boost;

bool isPAGEset = false;
bool isTITLEset = false;
bool isIDset = false;
bool isREVISIONset = false;
bool isTEXTset = false;
bool isUSERNAMEset = false;
bool isCONTRIBUTORset = false;
bool isMINORset = false;
bool isCOMMENTset = false;
bool isTIMESTAMPset = false;;

bool isINFOBOXset=false;
bool isLINKset=false;
bool isREFERENCEset=false;
bool isCATEGORYset=false;

int docWordCount=0;

vector<string> title;
vector<string> category;
vector<string> body;
vector<string> infodata;
vector<string> outlink;
vector<string> references;
string docID;
string remaining="";
ofstream indexfile;
ofstream doctitlefile;
//FILE *indexfile;
//FILE*doctitlefile;
map<string,int> stopWord;


map<string,map<string,int> > myDict;  /*for now just store the id of doc and tf  in which this word is*/


bool stop_word(string t){
    if(stopWord.find(t)!= stopWord.end())
        return true;
    return false;
}

  

MySaxParser::MySaxParser(): xmlpp::SaxParser()
{
}

MySaxParser::~MySaxParser()
{
}

void MySaxParser::on_start_document()
{
  std::cout << "on_start_document()" << std::endl;
}

void MySaxParser::on_end_document()
{
  std::cout << "on_end_document()" << std::endl;
}

void MySaxParser::on_start_element(const Glib::ustring& name,const AttributeList& attributes)
{
    string tagName=name;
//    std::cout << "node name=" << tagName << std::endl;
    
    if(!tagName.compare("title"))
        isTITLEset = true;
    if(!tagName.compare("id"))
        isIDset = true;
    if(!tagName.compare("revision"))
        isREVISIONset = true;
    if(!tagName.compare("timestamp"))
        isTIMESTAMPset = true;  
    if(!tagName.compare("contributor"))
        isCONTRIBUTORset = true;            
    if(!tagName.compare("minor"))
        isMINORset = true;  
    if(!tagName.compare("comment"))
        isCOMMENTset = true;    
    if(!tagName.compare("text"))
        isTEXTset = true;   
    if(!tagName.compare("page")){
        isPAGEset = true;
        docWordCount=0;
        title.clear();
        category.clear();
        body.clear();
        outlink.clear();
        infodata.clear();
        references.clear();
        isINFOBOXset=false;
        isLINKset=false;
        isREFERENCEset=false;
        isCATEGORYset=false;
    }
}

void MySaxParser::on_end_element(const Glib::ustring& name)
{
    string tagName=name.lowercase();
    //    stringstream ss;
    //    ss << name.lowercase();
    //    ss >> tagName;
    trim(tagName);
    //    std::cout << "on_end_element("<<tagName<<")"<< std::endl;
    if(!tagName.compare("title")){
        isTITLEset = false;
        //        cout<<"hello "<<tagName<<endl;
    }
    if(!tagName.compare("id"))
        isIDset = false;
    if(!tagName.compare("revision"))
        isREVISIONset = false;
    if(!tagName.compare("timestamp"))
        isTIMESTAMPset = false; 
    if(!tagName.compare("contributor"))
        isCONTRIBUTORset = false;           
    if(!tagName.compare("minor"))
        isMINORset = false; 
    if(!tagName.compare("comment"))
        isCOMMENTset = false;   
    if(!tagName.compare("text"))
        isTEXTset = false;  
    if(!tagName.compare("page")){
        isPAGEset = false;
        int size=0,varSize=0;
        string data;
        string ind;
        vector<string> var;
        size=title.size();
        doctitlefile<<docID<<":";
        //fprintf(doctitlefile,"%s:",docID);
        for(int i = 0 ; i < size ; i++){
            data = title[i];
            trim(data);
            //            trim_if(data,is_any_of(" \"\n\t\r"));
            if(!all(data,is_alpha())){
                if(data.size()>3)
                    data=data.substr(3);
            }
            if(data.size() > 2 && !stop_word(data) && all(data, is_alpha())){
                //                cout<<data<<"++++++++"<<endl;
                doctitlefile<<data<<" ";
                //fprintf(doctitlefile,"%s ",data);
                Porter2Stemmer::stem(data);
                //                cout<<data<<"++++++++"<<endl;
                if(!stop_word(data)){
                    ind=data+"-T";
                    //  myDict[ind].insert(docID);
                    myDict[ind][docID]++;
                    docWordCount++;
                    // myDict[data][docID]["T"]++;
                }
            }
        }
        //fprintf(doctitlefile,"\n");
        size=category.size();
        for(int i = 0 ; i < size ; i++){
            split(var,category[i],is_any_of("\\/#()![]{}@$%^&*_-+=~`<>:;\'\"?., |0123456789\n\r\t"),token_compress_on);
            varSize=var.size();
            for(int j = 0 ; j < varSize ; j++){
                data=var[j];
                trim(data);
                if(!all(data,is_alpha())){
                    if(data.size()>3)
                        data=data.substr(3);
                }
                //                trim_if(data,is_any_of(" \"\n\t\r"));
                if(data.size() > 2 && !stop_word(data) && all(data, is_alpha())){
                    //                    cout<<data<<"++++++++="<<endl;
                    Porter2Stemmer::stem(data);
                    //                    cout<<data<<"++++++++"<<endl;
                    if(!stop_word(data)){
                        ind=data+"-C";
                        //    myDict[ind].insert(docID);
                        myDict[ind][docID]++;
                        docWordCount++;
                        //myDict[data][docID]["C"]++;
                    }
                }

            }
        }
        var.clear();
        size=body.size();
        for(int i = 0 ; i < size ; i++){
            split(var,body[i],is_any_of("\\/#()[]{}!@$%^&*_-+=~`<>:;\'\"?., |0123456789\n\r\t"),token_compress_on);
            varSize=var.size();
            for(int j = 0 ; j < varSize ; j++){
                data=var[j];
                trim(data);
                if(!all(data,is_alpha())){
                    if(data.size()>3)
                        data=data.substr(3);
                }
                //                trim_if(data,is_any_of(" \"\n\t\r"));
                if(data.size() > 2 && !stop_word(data) && all(data, is_alpha())){
                    //                    cout<<data<<"++++++++"<<endl;
                    Porter2Stemmer::stem(data);
                    //                    cout<<data<<"++++++++"<<endl;
                    if(!stop_word(data)){
                        ind=data+"-B";
                        //  myDict[ind].insert(docID);
                        myDict[ind][docID]++;
                        docWordCount++;
                        //myDict[data][docID]["B"]++;
                    }
                }
            }
        }
        var.clear();
        size=infodata.size();
        for(int i = 0 ; i < size ; i++){
            split(var,infodata[i],is_any_of("\\/#()[]{}!@$%^&*_-+=~`<>:;\'\"?., |0123456789\n\t\r"),token_compress_on);
            varSize=var.size();
            for(int j = 0 ; j < varSize ; j++){
                data=var[j];
                trim(data);
                if(!all(data,is_alpha())){
                    if(data.size()>3)
                        data=data.substr(3);
                }
                //                trim_if(data,is_any_of(" \"\n\t\r"));
                if(data.size() > 2 && !stop_word(data) && all(data, is_alpha())){
                    //                    cout<<data<<"++++++++"<<endl;
                    Porter2Stemmer::stem(data);
                    //                    cout<<data<<"++++++++"<<endl;
                    if(!stop_word(data)){
                        ind=data+"-I";
                        //    myDict[ind].insert(docID);
                        myDict[ind][docID]++;
                        docWordCount++;
                        //myDict[data][docID]["I"]++;
                    }
                }
            }
        }
        var.clear();
        size=outlink.size();
        for(int i = 0 ; i < size ; i++){
            split(var,outlink[i],is_any_of("\\/#()[]{}!@$%^&*_-+=~`<>:;\'\"?., |0123456789\n\r\t"),token_compress_on);
            varSize=var.size();
            for(int j = 0 ; j < varSize ; j++){
                data=var[j];
                trim(data);
                if(!all(data,is_alpha())){
                    if(data.size()>3)
                        data=data.substr(3);
                }
                //                trim_if(data,is_any_of(" \n\t\r"));
                if(data.size() > 2 && !stop_word(data) && all(data, is_alpha())){
                    //                    cout<<data<<"++++++++"<<endl;
                    Porter2Stemmer::stem(data);
                    //                    cout<<data<<"++++++++"<<endl;
                    if(!stop_word(data)){
                        ind=data+"-O";
                        //    myDict[ind].insert(docID);
                        myDict[ind][docID]++;
                        docWordCount++;
                        //myDict[data][docID]["O"]++;
                    }
                }
            }
        }
        var.clear();
        size=references.size();
        for(int i = 0 ; i < size ; i++){
            split(var,references[i],is_any_of("\\/#()[]{}!@$%^&*_-+=~`<>:;\'\"?., |0123456789\n\r\t"),token_compress_on);
            varSize=var.size();
            for(int j = 0 ; j < varSize ; j++){
                data=var[j];
                trim(data);
                if(!all(data,is_alpha())){
                    if(data.size()>3)
                        data=data.substr(3);
                }
                //                trim_if(data,is_any_of(" \n\t\r"));
                if(data.size() > 2 && !stop_word(data) && all(data, is_alpha())){
                    //                    cout<<data<<"++++++++"<<endl;
                    Porter2Stemmer::stem(data);
                    //                    cout<<data<<"++++++++"<<endl;
                    if(!stop_word(data)){
                        ind=data+"-R";
                        //  myDict[ind].insert(docID);
                        myDict[ind][docID]++;
                        docWordCount++;
                        //myDict[data][docID]["R"]++;
                    }
                }
            }
        }
        map<string,map<string,int>  >::iterator it;
        map<string,int  > ::iterator it1;
        //      map<string,int >  ::iterator it2;
        size=myDict.size();
        if(size>5000){
            for (it=myDict.begin(); it!=myDict.end(); ++it){
                indexfile<<it->first<<":";
                for (it1=(it->second).begin(); it1!=(it->second).end(); ++it1){
                    indexfile<<it1->first<<"-"<<it1->second<<",";
                }
                indexfile<<endl;
            }
            myDict.clear();
        }
        doctitlefile<<":"<<docWordCount<<endl;
    }
}

void MySaxParser::on_characters(const Glib::ustring& text){
    try{
//        cout << "on_characters(): " << text<<"----------"<< endl;
        //        stringstream ss;
        string textStream;
        string middleStr=text.lowercase();
//       trim(middleStr);
        size_t middleStrFind=middleStr.find_last_of(" ");
        if(middleStrFind!=string::npos){
            textStream=remaining+middleStr.substr(0,middleStrFind);
            remaining=middleStr.substr(middleStrFind);
        }else{
            textStream=remaining+middleStr;
            remaining="";
        }
        trim(textStream);
//        cout << "on_characters(): " << textStream<<"++++++++++++"<< endl;
        if(isTITLEset){
            split(title, textStream, is_any_of("\\/#()[]{}!@$%^&*_-+=~`<>:;\'\"?., |0123456789\n\r\t"),token_compress_on ); 
        }else if(!isREVISIONset && isIDset){
            docID = textStream;
            trim(docID);
        }else if(isTEXTset){
            if(textStream.find("{{infobox") != string::npos)
                isINFOBOXset=true;

            if( textStream.find("|footnotes") !=  string::npos || textStream.find("}}")==0 )
                isINFOBOXset=false;

            if(isINFOBOXset){
                vector<string> temp;
                split(temp, textStream, is_any_of("|"));
                int tempSize =temp.size();
                for(int i=0;i<tempSize;i++){
                    string token=temp[i];
                    trim(token);
                    if(token!=""){
                        vector<string> pair;
                        split(pair,token,is_any_of("="));
                        if(pair.size()==2){
                            infodata.push_back(pair[0]);
                            string rhs=pair[1];
                            trim(rhs);
                            if(rhs!=""){
                                size_t rhsFind=rhs.find("[[");
                                if(rhsFind!= string::npos){
                                    isLINKset=true;
                                    if(rhsFind==0){
                                        string x=rhs.substr(2);
                                        size_t xFind=x.find("]]");
                                        if(xFind!=string::npos){
                                            isLINKset=false;
                                            if(xFind==(x.size()-2)){
                                                string y=x.substr(0,x.size()-2);
                                                outlink.push_back(y);
                                            }else{
                                                string y=x.substr(0,xFind);
                                                string z=x.substr(xFind+2);
                                                outlink.push_back(y);
                                                infodata.push_back(z);
                                            }
                                        }else{
                                            outlink.push_back(x);
                                        }

                                    }else{
                                        string x=rhs.substr(0,rhsFind);
                                        string y=rhs.substr(rhsFind+2);
                                        infodata.push_back(x);
                                        size_t yFind=y.find("]]");
                                        if(yFind!=string::npos){
                                            isLINKset=false;
                                            if(yFind==(y.size()-2)){
                                                string z=y.substr(0,y.size()-2);
                                                outlink.push_back(z);
                                            }else{
                                                string u=y.substr(0,yFind);
                                                string z=y.substr(yFind+2);
                                                outlink.push_back(u);
                                                infodata.push_back(z);
                                            }
                                        }else{
                                            outlink.push_back(y);
                                        }
                                    }
                                }else{
                                    infodata.push_back(rhs);
                                }
                            }else{
                                /* Empty string :-( */
                            }
                        }else{
                            if(isLINKset){
                                size_t tokenFind=token.find("]]");
                                if(tokenFind!= string::npos){
                                    isLINKset=false;
                                    if(tokenFind==(token.size()-2)){
                                        string z=token.substr(0,token.size()-2);
                                        outlink.push_back(z);
                                    }else{
                                        string y=token.substr(0,tokenFind);
                                        string z=token.substr(tokenFind+2);
                                        outlink.push_back(y);
                                        infodata.push_back(z);
                                    }
                                }else{
                                    outlink.push_back(token) ;
                                }
                            }else{
                                size_t tokenFind=token.find("[[");
                                if(tokenFind!= string::npos){
                                    isLINKset=true;
                                    if(tokenFind==0){
                                        string x=token.substr(2);
                                        size_t xFind=x.find("]]");
                                        if(xFind!=string::npos){
                                            isLINKset=false;
                                            if(xFind==(x.size()-2)){
                                                string y=x.substr(0,xFind);
                                                outlink.push_back(y);
                                            }else{
                                                string y=x.substr(0,xFind);
                                                string z=x.substr(xFind+2);
                                                outlink.push_back(y);
                                                infodata.push_back(z);
                                            }
                                        }else{
                                            outlink.push_back(x);
                                        }

                                    }else{
                                        string x=token.substr(0,tokenFind);
                                        string y=token.substr(tokenFind+2);
                                        infodata.push_back(x);
                                        size_t yFind=y.find("]]");
                                        if(yFind!=string::npos){
                                            isLINKset=false;
                                            if(yFind==(y.size()-2)){
                                                string z=y.substr(0,yFind);
                                                outlink.push_back(z);
                                            }else{
                                                string u=y.substr(0,yFind);
                                                string z=y.substr(yFind+2);
                                                outlink.push_back(u);
                                                infodata.push_back(z);
                                            }
                                        }else{
                                            outlink.push_back(y);
                                        }
                                    }
                                }else{
                                    infodata.push_back(token);
                                    /*What to do here is seems either this field is empty or there are some issues with the format */
                                }
                            }
                        }
                    }else{
                        /* token is empty so do nothing take next token */
                    }
                }
            }else{
                vector<string> temp;
                split(temp, textStream, is_any_of(" "));
                int tempSize =temp.size();
                for(int i=0;i<tempSize;i++){
                    string token=temp[i];
                    trim(token);
                    if(token!=""){
                      //  cout<<"dkjhfvdhfw"<<endl;
                        size_t tokenFind=token.find("=references=");
                        if(isREFERENCEset || tokenFind != string::npos ){
                            if(tokenFind != string::npos){
                                isREFERENCEset=true;
                            }
                            references.push_back(token);
                            tokenFind=token.find("=external links=");
                            if(tokenFind != string::npos){
                                isREFERENCEset=false;
                            }
                        }else{
                      //  cout<<"1234"<<endl;
                            tokenFind=token.find("[[category");
                            if(isCATEGORYset || tokenFind != string::npos ){
                                if(tokenFind != string::npos){
                                    isCATEGORYset = true;
                                    if(tokenFind==0){
                                        string x=token.substr(10);
                                        size_t xFind=x.find("]]");
                                        if(xFind!=string::npos){
                                            isCATEGORYset=false;
                                            if(xFind==(x.size()-2)){
                                                string y=x.substr(0,x.size()-2);
                                                category.push_back(y);
                                            }else{
                                                string y=x.substr(0,xFind);
                                                string z=x.substr(xFind+2);
                                                category.push_back(y);
                                                body.push_back(z);
                                            }
                                        }else{
                                            category.push_back(x);
                                        }
                                    }else{
                                        string x=token.substr(0,tokenFind);
                                        string y=token.substr(tokenFind+10);
                                        body.push_back(x);
                                        size_t yFind=y.find("]]");
                                        if(yFind!=string::npos){
                                            isCATEGORYset=false;
                                            if(yFind==(y.size()-2)){
                                                string z=y.substr(0,y.size()-2);
                                                category.push_back(z);
                                            }else{
                                                string u=y.substr(0,yFind);
                                                string z=y.substr(yFind+2);
                                                category.push_back(u);
                                                body.push_back(z);
                                            }
                                        }else{
                                            outlink.push_back(y);
                                        }
                                    }
                                }else{
                                    size_t uFind=token.find("]]");
                                    if(uFind != string::npos){
                                        isCATEGORYset = false;
                                        if(uFind==(token.size()-2)){
                                            string x=token.substr(0,uFind);
                                            category.push_back(x);
                                        }else{
                                            string y=token.substr(0,uFind);
                                            string z=token.substr(uFind+2);
                                            category.push_back(y);
                                            body.push_back(z);
                                        }
                                    }else{
                                        category.push_back(token);
                                    }
                                }
                            }else{
                                /*CAREGORY not set */
                                /* check for outlink */
                           //     cout<<"outlink"<<endl;
                                size_t vFind=token.find("[");
                                if(vFind!= string::npos){
                             //       cout<<"outlinkconf"<<endl;
                                    isLINKset=true;
                                    if(vFind==0){
                               //         cout<<"outlinkconf1"<<endl;
                                        string x=token.substr(1);
                                 //       cout<<"here3"<<endl;
                                        size_t xFind=x.find("]");
                                        if(xFind!=string::npos){
                                   //         cout<<"here"<<endl;
                                            isLINKset=false;
                                            if(xFind==(x.size()-1)){
                                                string y=x.substr(0,xFind);
                                                outlink.push_back(y);
                                            }else{
                                                string y=x.substr(0,xFind);
                                                string z=x.substr(xFind+1);
                                                outlink.push_back(y);
                                                body.push_back(z);
                                            }
                                        }else{
                                            outlink.push_back(x);
                                        }
                                     //   cout<<"hello"<<endl;
                                    }else{
                                       // cout<<"outlinkconf2"<<endl;
                                        string x=token.substr(0,vFind);
                                        string y=token.substr(vFind+1);
                                        infodata.push_back(x);
                                        size_t yFind=y.find("]");
                                        if(yFind!=string::npos){
                                            isLINKset=false;
                                            if(yFind==(y.size()-1)){
                                                string z=y.substr(0,yFind);
                                                outlink.push_back(z);
                                            }else{
                                                string u=y.substr(0,yFind);
                                                string z=y.substr(yFind+1);
                                                outlink.push_back(u);
                                                body.push_back(z);
                                            }
                                        }else{
                                            outlink.push_back(y);
                                        }
                                    }
                                }else{
                                    size_t wFind=token.find("]");
                                    if(wFind!=string::npos){
                                        isLINKset=false;
                                        if(wFind==(token.size()-1)){
                                            string z=token.substr(0,wFind);
                                            outlink.push_back(z);
                                        }else{
                                            string u=token.substr(0,wFind);
                                            string z=token.substr(wFind+1);
                                            outlink.push_back(u);
                                            body.push_back(z);
                                        }
                                    }else{
                                        if(isLINKset){
                                            outlink.push_back(token);
                                        }else{
                                            body.push_back(token);
                                        }
                                    }
                                }
                            }
                        }
                    }else{
                        /* token is empty so do nothing take next token */
                    }
                }
            }
        }else{
            /*rest of the stuff like contributer timestamp */
        }

    }catch(const Glib::ConvertError& ex){
        std::cerr << "MySaxParser::on_characters(): Exception caught while converting text for std::cout: " << ex.what() << std::endl;
    }
}

void MySaxParser::on_comment(const Glib::ustring& text)
{
    try
    {
    std::cout << "on_comment(): " << text << std::endl;
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << "MySaxParser::on_comment(): Exception caught while converting text for std::cout: " << ex.what() << std::endl;
  }
}

void MySaxParser::on_warning(const Glib::ustring& text)
{
  try
  {
    std::cout << "on_warning(): " << text << std::endl;
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << "MySaxParser::on_warning(): Exception caught while converting text for std::cout: " << ex.what() << std::endl;
  }
}

void MySaxParser::on_error(const Glib::ustring& text)
{
  try
  {
    std::cout << "on_error(): " << text << std::endl;
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << "MySaxParser::on_error(): Exception caught while converting text for std::cout: " << ex.what() << std::endl;
  }
}

void MySaxParser::on_fatal_error(const Glib::ustring& text)
{
  try
  {
    std::cout << "on_fatal_error(): " << text << std::endl;
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << "MySaxParser::on_characters(): Exception caught while converting value for std::cout: " << ex.what() << std::endl;
  }
}


int
main(int argc, char* argv[])
{
  // Set the global C and C++ locale to the user-configured locale,
  // so we can use std::cout with UTF-8, via Glib::ustring, without exceptions.
  std::locale::global(std::locale(""));

  std::string filepath;
  if(argc > 1 ){
    filepath = argv[1]; //Allow the user to specify a different XML file to parse.
  }else{
      cout<<"Please input a xml file"<<endl;
      exit(0);
  }
  /* File to which we will write index */
  string indexPath=string(argv[2]);
  string indexFilePath=indexPath+"/index.txt";

  indexfile.open(indexFilePath,ios::trunc);
  if (!indexfile.is_open()){
      cout<<"error while opening the file index.txt"<<endl;
      exit(0);
  }    
  string docTitleFilePath=indexPath+"/docTitle.txt";
  doctitlefile.open(docTitleFilePath,ios::trunc);
  if (!doctitlefile.is_open()){
      cout<<"error while opening the file docTitle.txt"<<endl;
      exit(0);
  }    

  string line;
  ifstream myfile(argv[3]);
  if (myfile.is_open()){
      while ( getline (myfile,line) ){
   //       trim_if(line,is_any_of(" \\n\\t\\r"));
          stopWord[line]=1;
      }
      myfile.close();
  }else{
      cout<<"Error opening stopwords.txt"<<endl;
      exit(0);
  }

  // Parse the entire document in one go:
  int return_code = EXIT_SUCCESS;

  // Incremental parsing, sometimes useful for network connections:
  try
  {
    std::cout << std::endl << "Incremental SAX Parser:" << std::endl;
    
    std::ifstream is(filepath.c_str());
    if (!is)
      throw xmlpp::exception("Could not open file " + filepath);

    char buffer[64];
    const size_t buffer_size = sizeof(buffer) / sizeof(char);

    //Parse the file:
    MySaxParser parser;
    parser.set_substitute_entities(true);
    do
    {
      std::memset(buffer, 0, buffer_size);
      is.read(buffer, buffer_size-1);
      if(is.gcount())
      {
        // We use Glib::ustring::ustring(InputIterator begin, InputIterator end)
        // instead of Glib::ustring::ustring( const char*, size_type ) because it
        // expects the length of the string in characters, not in bytes.
        Glib::ustring input(buffer, buffer+is.gcount());
        parser.parse_chunk(input);
      }
    }
    while(is);

    parser.finish_chunk_parsing();
  }
  catch(const xmlpp::exception& ex)
  {
    std::cerr << "Incremental parsing, libxml++ exception: " << ex.what() << std::endl;
    return_code = EXIT_FAILURE;
  }
  map<string,map<string,int>  >::iterator it;
  map<string,int > ::iterator it1;
  for (it=myDict.begin(); it!=myDict.end(); ++it){
      indexfile<<it->first<<":";
      for (it1=(it->second).begin(); it1!=(it->second).end(); ++it1){
          indexfile<<it1->first<<'-'<<it1->second<<",";
      }
      indexfile<<endl;
  }
  myDict.clear();
  indexfile.close();
  doctitlefile.close();
  return return_code;
}
