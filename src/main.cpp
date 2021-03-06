#include <iostream>
#include <cstdio>
#include <map>

#include "program.h"
#include "reaching_defs.h"
#include "ssa.h"
#include "copy_propagation.h"
#include "cons_propagation.h"

using namespace std;

int main(int argc, char **argv){
  char key[32],val[32];
  bool debug=false;
  map<string,string> opts = {{"out","3addr"},{"opt","none"},{"help","false"},{"debug","false"}};
  for(int i=1;i<argc;i++){
    if(sscanf(argv[i],"%[^=]=%s",key,val)==2){
      opts[key] = val;
    }else{
      opts[argv[i]] = "true";
    }
  }
  if(opts["debug"] == "true")
    debug=true;
  if(debug){
    cerr<<"Using options: ";
    for(auto &opt: opts){
      cerr<<opt.first<<"="<<opt.second<<"; ";
    }
    cerr<<endl;
  }
  if(opts["help"] == "true"){
    cout<<"Usage: ./opt out=(3addr|cfg) opt=(none|reach|copy|ssa|const) [help] [debug] < input_file.3addr"<<endl;
    return 0;
  }
  Program program;

  if(opts["opt"].find("reac")==0){
    ReachingDefs rd(&program);
    rd.run(true);
  }else if(opts["opt"].find("copy")==0){
    CopyPropagation cp(&program);
    cp.run(true);
  }else if(opts["opt"].find("ssa")==0){
    SSA ssa(&program);
    ssa.run(true);
  }else if(opts["opt"].find("cons")==0){
    ConsPropagation cp(&program);
    cp.run(true);
  }else if(opts["opt"].find("none")!=0)
    cerr<<"Unsupported optimization/analysis: "<<opts["opt"]<<endl;

  program.schedule(2);

  if(opts["out"].find("3add")==0)
    program.emit();
  else if(opts["out"].find("cfg")==0)
    program.emitCFG();
  else
    cerr<<"Unsupported output format: "<<opts["out"]<<endl;
  return 0;
}
