#pragma once
#include "program.h"
#include "reaching_defs.h"
#include "instruction.h"
#include "register.h"
#include "value.h"
#include "cfg.h"
#include <algorithm>
class SSA{
public:
  // Class specific
  Program *prog;

  // Function specific member variables
  int id;
  CFG *cfg;
  set<Register*> regs;
  set<BasicBlock*> idf;
  set<BasicBlock*> visited_blocks;
  map<Register*, set<Instruction*> >  reg_insts;
  map<Register*, set<BasicBlock*> > reg_blocks;
  map<BasicBlock*, vector<Register*> > phis;
  map<BasicBlock*, Register* > moves;
  map<BasicBlock*, set<BasicBlock*> > tree;

  SSA(Program *_prog):prog(_prog){
  }
  void run(bool populate_meta=false){
    for(auto &inst_fn: prog->functions){
      processFunction(&(inst_fn.second));
    }
  }
  void clear(){
    id=0;
    cfg=NULL;
    regs.clear();
    idf.clear();
    reg_insts.clear();
    reg_blocks.clear();
  }
  void createMaps(Function* fn){
    clear();
    cfg = &(fn->cfg);
    for(auto &bb:cfg->blocks){
      for(auto i=bb->leader;i!=NULL;i=i->next){
        if(i->type!=Instruction::imove)
          continue;
        auto reg = i->op2.reg;
        if(regs.find(reg)==regs.end()){
          regs.insert(reg);
          reg_insts[reg]={};
          reg_blocks[reg]={};
        }
        reg_insts[reg].insert(i);
        reg_blocks[reg].insert(bb);
      }
    }
  }
  void createVirtualPhiMoves(){
    for(auto &bb:cfg->blocks){
      phis[bb] = {};
      phis[bb].assign(bb->getPred().size(), NULL);
      moves[bb] = NULL;
    }
  }
  void createDominatorTree(){
    for(auto &bb:cfg->blocks)
      tree[bb]={};
    for(auto p:cfg->idoms){
      if(p.second)
        tree[p.second].insert(p.first);
    }
  }
  void processFunction(Function *fn){
    // CFG
    createMaps(fn);
    createVirtualPhiMoves();
    createDominatorTree();
    // modifyDefInstructions();
    // Creating iterated dominance frontier for each variable
    for(auto reg:regs){
      visited_blocks.clear();
      id = 0;
      idf = cfg->idf(util::set_union(reg_blocks[reg], {cfg->root}));

      Register* running_reg = Register::allocVar(reg->name);
      running_reg->convert(id++);
      DFS(cfg->root, reg, running_reg);

      for(auto &bb:idf){
        insertAll(bb);
      }
    }
  }
  void DFS(BasicBlock* block, Register *reg, Register *running_reg){
    if(visited_blocks.find(block)!=visited_blocks.end())
      return;
    visited_blocks.insert(block);
    // Virtual phi nodes take care
    if(idf.find(block)!=idf.end()){
      moves[block] = (Register::allocVar(reg->name));
      moves[block]->convert(id++);
      running_reg = moves[block];
    }
    // Running instruction and register
    for(auto i=block->leader;i!=NULL;i=i->next){
      if(i->type == Instruction::imove){
        if(i->op1.reg==reg)
          i->op1.init(running_reg);
        if(i->op2.reg == reg){
          i->op2.init(Register::allocVar(reg->name));
          i->op2.reg->convert(id++);
          running_reg = i->op2.reg;
        }
      }
      else{
        if(i->op1.reg==reg)
          i->op1.init(running_reg);
        if(i->op2.reg==reg)
          i->op2.init(running_reg);
      }
    }
    // percolate register to successors
    for(auto &bb:block->getSucc()){
      if(idf.find(bb)!=idf.end()){
        auto pred = bb->getPred();
        int i = distance(pred.begin(), pred.find(block));
        phis[bb][i] = running_reg;
      }
    }
    // DFS
    for(auto &bb:tree[block]){
      DFS(bb, reg, running_reg);
    }
  }
  void insertPhi(BasicBlock* block, Register* op1_reg, Register* op2_reg, int p1, int p2){
    Instruction *phi = Instruction::alloc();
    phi->type = Instruction::iphi;
    if(op1_reg)
      phi->op1.init(op1_reg);
    phi->op2.init(op2_reg);
    phi->phi_parent1 = p1;
    phi->phi_parent2 = p2;

    Instruction* leader = block->leader;
    leader->op1.init(phi->out);

    swap(*leader, *phi);
    swap(leader, phi);
    phi->next = leader;
  }
  void insertMove(BasicBlock* block, Register* op2_reg){
    Instruction* move = Instruction::alloc();
    move->type = Instruction::imove;
    move->op2.init(op2_reg);

    Instruction* leader = block->leader;
    swap(*leader, *move);
    swap(leader, move);
    move->next = leader;
  }
  void insertAll(BasicBlock* block){
    for(auto x:phis[block])
      assert(x!=NULL);
    assert(moves[block]!=NULL);
    // Inserting all phi instructions
    insertMove(block, moves[block]);
    for(int i=phis[block].size()-1;i>=2;i--){
      insertPhi(block, NULL, phis[block][i], -1, i);
    }
    insertPhi(block, phis[block][0], phis[block][1], 0, 1);
  }
};
