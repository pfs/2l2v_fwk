#include "UserCode/llvv_fwk/interface/JSONWrapper.h"

using namespace std;

namespace JSONWrapper{

  std::string removeWhiteSpace(const std::string& in){
    int n = in.size();
    std::string out = "";
     
    bool isText1 = false;
    bool isText2 = false;
    for(int i=0;i<n;i++){
      if(in[i]=='\"')isText1=!isText1;
      if(in[i]=='\'')isText2=!isText2;

      if(!isText1 && !isText2 && (in[i]==' ' || in[i]=='\n' || in[i]=='\t' || in[i]=='\r'))continue;
      out += in[i];  
    } 
    return out;
  }


  std::string removeQuotes(const std::string& in){
    int n = in.size();
    std::string out = "";
    if(in[0]=='\"' && in[n-1]=='\"')return in.substr(1,n-2);
    return in;
  }


  size_t findComma(const std::string& in, int start){
    int n = in.size();
    bool isText1 = false;
    bool isText2 = false;
    int  isBracket = 0;
    int  isBrace   = 0;
    for(int i=start;i<n;i++){
      if(in[i]=='\"')isText1=!isText1;
      if(in[i]=='\'')isText2=!isText2;
      if(in[i]=='[')isBracket++;
      if(in[i]==']')isBracket--;
      if(in[i]=='{')isBrace  ++;
      if(in[i]=='}')isBrace  --;

      if(!isText1 && !isText2 && isBracket==0 && isBrace  ==0 && in[i]==',' )return i;
    }
    return std::string::npos;
  }


  size_t findEndBrace(const std::string& in, int start){
    int n = in.size();
    bool isText1 = false;
    bool isText2 = false;
    int  isBracket = 0;
    int  isBrace   = 1;
    for(int i=start;i<n;i++){
      if(in[i]=='\"')isText1=!isText1;
      if(in[i]=='\'')isText2=!isText2;
      if(in[i]=='[')isBracket++;
      if(in[i]==']')isBracket--;
      if(in[i]=='{')isBrace  ++;
      if(in[i]=='}')isBrace  --;

      if(!isText1 && !isText2 && isBracket==0 && isBrace  ==0)return i;
    }
    return std::string::npos;
  }

  size_t findEndBracket(const std::string& in, int start){
    int n = in.size();
    bool isText1 = false;
    bool isText2 = false;
    int  isBracket = 1;
    int  isBrace   = 0;
    for(int i=start;i<n;i++){
      if(in[i]=='\"')isText1=!isText1;
      if(in[i]=='\'')isText2=!isText2;
      if(in[i]=='[')isBracket++;
      if(in[i]==']')isBracket--;
      if(in[i]=='{')isBrace  ++;
      if(in[i]=='}')isBrace  --;

      if(!isText1 && !isText2 && isBracket==0 && isBrace  ==0)return i;
    }
    return std::string::npos;
  }


  bool isObject(const std::string& in){return in[0]=='{';}
  bool isArray (const std::string& in){return in[0]=='[';}
  bool isComma (const std::string& in){return in[0]==',';}


  Object::Object(const std::string& in, bool IsInputFile){
    if(IsInputFile){
      Load(in);
    }else{
      ParseObject(in);
    }
  }

  void Object::Load(const std::string& in){
    printf("Loading: %s...", in.c_str());fflush(stdout);
    FILE* pFile = fopen(in.c_str(),"r");

    char buffer[4096];
    std::string JsonFile = "";
    while(!feof(pFile)){
      fgets(buffer,4096,pFile);
      JsonFile += buffer;
    }
    fclose(pFile);
    JsonFile = removeWhiteSpace(JsonFile);
    ParseObject(JsonFile);
    printf("  Done\n");fflush(stdout);
  }


  void Object::ParseObject(const std::string& in){
    if(isObject(in)){
      GetObjects(in);
      val = "LIST";
    }else if(isArray(in)){
      GetArray(in);
      val = "ARRAY";
    }else{
      val = removeQuotes(in);
    }
  }

  void Object::GetObjects(const std::string& in){
    size_t start   = in.find('{');
    size_t mid     = in.find(':', start);
    size_t end     = findEndBrace(in,mid);//in.rfind('}');
    size_t nextval = std::min(findComma(in,mid),end);

    do{
      key.push_back( removeQuotes(in.substr( start+1,(mid-start)-1) )    );
      obj.push_back( Object(in.substr( mid  +1,  (nextval-mid)-1) ) );    

      start   = nextval;
      mid     = in.find(':', start);
      nextval = std::min(findComma(in,mid),end);

      if(start==nextval || nextval<=mid)break;
    }while(nextval<=end);
  }


  void Object::GetArray(const std::string& in){
    size_t start   = in.find('[');
    size_t end     = findEndBracket(in,start+1);
    size_t nextval = std::min(findComma(in,start+1),end);

    do{
      key.push_back("obj" );
      obj.push_back( Object(in.substr( start+1,  (nextval-start)-1) ) );

      start   = nextval;
      nextval = std::min(findComma(in,start+1),end);
      if(start==nextval)break;
    }while(nextval<=end);
  }


  void Object::Print(int Level){
    std::string indent = "";for(int i=0;i<Level;i++)indent += "   ";

    for(unsigned int i=0;i<key.size();i++){
      printf("%sKEY = %20s   VAL = %s Ndaughters=%i\n",indent.c_str(),key[i].c_str(), obj[i].val.c_str(), (int)obj[i].key.size());
      obj[i].Print(Level+1);
    }
  } 


  void Object::Dump(char* pFile, int Level, bool whitespace){
      
    if(Level==0){
      sprintf(pFile, "%s{",pFile); if(whitespace)sprintf(pFile,"%s\n",pFile);        
      Dump(pFile, Level+1);      
      sprintf(pFile, "%s}",pFile);    
      return;   
    }

    std::string indent = "";for(int i=0;i<Level && whitespace;i++)indent += "  ";
    for(int i=0;i<(int)key.size();i++){
      string keyI=key[i].c_str();
      string valI=obj[i].val.c_str();
      string valITxt=valI;
      if(obj[i].isString())valITxt = string("\"") + valITxt + "\"";

      if(valI=="LIST"){
	sprintf(pFile,"%s%s{", pFile,indent.c_str()); if(whitespace)sprintf(pFile,"%s\n",pFile);
	obj[i].Dump(pFile, Level+1);
	sprintf(pFile,"%s%s}", pFile,indent.c_str());
      }else if(valI=="ARRAY"){
	sprintf(pFile,"%s%s\"%s\":[", pFile,indent.c_str(), keyI.c_str()); if(whitespace)sprintf(pFile,"%s\n",pFile);
	obj[i].Dump(pFile, Level+1);
	sprintf(pFile,"%s%s]", pFile,indent.c_str());
      }else if(keyI=="obj"){
	sprintf(pFile, "%s%s%s",pFile,indent.c_str(), valITxt.c_str());
      }else{
	sprintf(pFile, "%s%s\"%s\":%s",pFile,indent.c_str(),keyI.c_str(), valITxt.c_str());
      }

      if(i<((int)key.size())-1){
	sprintf(pFile, "%s,",pFile);
      }
      if(whitespace)sprintf(pFile,"%s\n",pFile);
    }
  }

  void Object::Dump(FILE* pFile, int Level){
    char out[100000] = "";
    Dump(out, Level, true);
    fprintf(pFile, "%s", out);
  }

  string Object::DumpToString(int Level){
    char out[100000] = "";
    Dump(out, Level, false);
    return string(out);
  }



  /*
    void Object::Dump(FILE* pFile, int Level){
    if(Level==0){
    fprintf(pFile, "{\n");        
    Dump(pFile, Level+1);      
    fprintf(pFile, "}");    
    return;   
    }

    std::string indent = "";for(int i=0;i<Level;i++)indent += "  ";
    for(int i=0;i<(int)key.size();i++){
    string keyI=key[i].c_str();
    string valI=obj[i].val.c_str();
    string valITxt=valI;
    if(obj[i].isString())valITxt = string("\"") + valITxt + "\"";

    if(valI=="LIST"){
    fprintf(pFile,"%s{\n", indent.c_str());
    obj[i].Dump(pFile, Level+1);
    fprintf(pFile,"%s}", indent.c_str());
    }else if(valI=="ARRAY"){
    fprintf(pFile,"%s\"%s\":[\n", indent.c_str(), keyI.c_str());
    obj[i].Dump(pFile, Level+1);
    fprintf(pFile,"%s]", indent.c_str());
    }else if(keyI=="obj"){
    fprintf(pFile, "%s%s",indent.c_str(), valITxt.c_str());
    }else{
    fprintf(pFile, "%s\"%s\":%s",indent.c_str(),keyI.c_str(), valITxt.c_str());
    }

    if(i<((int)key.size())-1){
    fprintf(pFile, ",");
    }
    fprintf(pFile, "\n");         
    }
    }
  */
}

