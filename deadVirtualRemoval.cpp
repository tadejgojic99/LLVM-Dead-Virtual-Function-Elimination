#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/PassInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/CallGraph.h"

using namespace llvm;


/*
  Podesavanje llvm-project preko skripte sa strane kursa (Vec build-ovan llvm-projekat -> to je stara verzija): 

  1) Moramo promeniti putanje u MakeFile i CMakeCache.txt 

  - default putanja podrazumeva da je llvm-project postavljen na desktop-u
  - Mozemo promeniti cele putanje i staviti projekat gde hocemo ili samo promeniti username u svim putanjama
    njegova vm putanja u projektu je home/---> user <--- sporni deo/Desktop/...
    samo treba uraditi replace all nad user na username koji je na nasem racunaru. 
    
    Na fajlovima:
    a) .../llvm-project/build/Makefile
    b) .../llvm-project/build/CmakeCache.txt

  2) Pokrenuti kompilaciju sa make (Prva kompilacija ce verovatno trajati jako dugo)
*/

/*
  Instrukcije za pokretanje:
  --------------------------

  1)  Fajl nad kojim hocemo da testiramo nas pass prevodimo u .ll:
      clang -S -O0 -emit-llvm ___.cpp
  ---------------------------------------------------

  2) Primer pokretanja pass-a sa pozicijom u llvm-project/build/lib
      ./../bin/opt -load LLVMDeadVirtual.so -enable-new-pm=0 -DVFE /home/.....Put do fajla 1)... 

*/ 

namespace {

// Definisemo pass: buduci da je u pitanju eliminacija mrtvih virtuelnih funkcija, nasledjujemo ModulePass
class DeadVirtualFunctionEliminationPass : public ModulePass {
public:
  static char ID;
  CallGraph *CG;
  DeadVirtualFunctionEliminationPass() : ModulePass(ID) {}

  // Sam ulaz Optimization Pass-a, kao argument prima modul
  bool runOnModule(Module &M) override {
    CG = &getAnalysis<CallGraphWrapperPass>().getCallGraph();
    bool Changed = false;  // Provera: ukoliko se bude transformisao kod, tj. ukoliko se eliminise mrtva virtuelna funkcija, postace true

    for (Function &F : M) {
      if (!F.isDeclaration() && !F.isIntrinsic()) {
        // Za svaku funkciju iz modula proveravamo da li zadovoljava uslov neke od ovih funkcija. Ako da, brisemo je, jer ukoliko zadovoljava bilo koju od ovih funkcija, eliminacija ne remeti rad modula
        if (isUnreachableVirtualFunction(F)) {
          F.deleteBody();
          Changed = true;
        }
      }
    }

    return Changed;
  }






  void getAnalysisUsage(AnalysisUsage &AU) const {
    // Oznacavamo da se analiza CallGraphWrapperPass izvrsi pre pass-a
    AU.addRequired<CallGraphWrapperPass>();
  }

private:
  // Ovde definisemo te funkcije koje su nam kriterijumi za identifikaciju mrtvih virtuelnih funkcija

  bool isCalledFromFunction(const Function* F, const Function* Caller) {
    // Prica sa vezbi BB -> Instructions, iteriramo kroz instrukcije instrukkcije Caller funkcije 
    for (const BasicBlock &BB : *Caller) {      // *Caller
        for (const Instruction &Inst : BB) {
            // Proveravamo da li je instrukcija 'call'
            if (const CallInst *CI = dyn_cast<CallInst>(&Inst)) {
                // Uzimamo pozvanu funkciju iz instrukcije
                const Function *CalledFunc = CI->getCalledFunction();

                if (CalledFunc && CalledFunc == F) {
                    return true;
                }
            }
        }
    }

    return false;
  }

  // 1. Nedostizne virtuelne funkcije
  bool isUnreachableVirtualFunction(const Function &F) {
  CallGraphNode* CGNode = (*CG)[&F];

    for (const auto &I : F) {    // *F
        if (const CallInst *CInst = dyn_cast<CallInst>(&I)) {
            const Function *Callee = CInst->getCalledFunction();
            if (Callee && Callee->isDeclaration()) {
                //Ako je pozvana funkcija deklaracija, nastavljamo dalje samo
                continue;
            }
            if (isCalledFromFunction(&F,Callee)) {
                // Ako nemamo granu poziva iz trenutne funkcije F u pozvanu funkciju Callee, onda je virtuelna funkcija nedostizna
                return true;
            }
        }
    }
    // Ako su sve grane poziva iz trenutne funkcije prisutne u pozvanim funkcijama, onda je virtuelna funkcija dostizna
    return false;  	
  }

};   // kraj namespace-a 


char DeadVirtualFunctionEliminationPass::ID = 0;
static RegisterPass<DeadVirtualFunctionEliminationPass> X("DVFE",
                                                         "Dead Virtual Function Elimination Pass",
                                                         false, false);
}