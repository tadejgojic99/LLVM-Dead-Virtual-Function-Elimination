#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Type.h"
#include "llvm/PassInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Support/Debug.h"
//----------------------------------------
#include <vector>
#include <regex>

using namespace llvm;

namespace {

// Definisemo pass: buduci da je u pitanju eliminacija mrtvih virtuelnih funkcija, nasledjujemo ModulePass
class DeadVirtualFunctionEliminationPass : public ModulePass {
public:
  static char ID;
  CallGraph *CG;
  DeadVirtualFunctionEliminationPass() : ModulePass(ID) {}
  
  bool runOnModule(Module &M) override {
    CG = &getAnalysis<CallGraphWrapperPass>().getCallGraph();
    bool Changed = false;  // Provera: ukoliko se bude transformisao kod, tj. ukoliko se eliminise mrtva virtuelna funkcija, postace true

    for (Function &F : M) {

      if (!F.isDeclaration() && !F.isIntrinsic()) {
        // Za svaku funkciju iz modula proveravamo da li zadovoljava uslov neke od ovih funkcija. Ako da, brisemo je, jer ukoliko zadovoljava bilo koju od ovih funkcija, eliminacija ne remeti rad modula
          if(    isAdequateFunction(F) 
              || isUnusedVirtualFunction(F)
              || isUnreachableVirtualFunction(F) 
              || hasNoImplementedVirtualFunction(F) 
              || hasConstantReturnVirtualFunction(F)) 
              {
                
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

// Pomocna f-ja 
// Proveramo da li je f-ja virtuelna na osnovu (virtual_) tj svaka f-ja u svom nazivu mora imati virtual_
// Primer virtual_<ime_fje> ili <ime_fje>_virtual
bool isVirtual(Function& F){
    return std::regex_match(F.getName().str(),std::regex("(.*)_+virtual_+(.+)"))?true:false; 
}

// Ovde definisemo te funkcije koje su nam kriterijumi za identifikaciju mrtvih virtuelnih funkcija
bool hasMatchingArgumentTypes(const Function &F, const std::vector<Type*> &expectedTypes) {
if (F.arg_size() != expectedTypes.size()) {
    return false;  // Razlicit broj argumenata
}

auto it = expectedTypes.begin();
for (const Argument &Arg : F.args()) {
    if (Arg.getType() != *it) {
    return false;  // Razlicit tip argumenata
    }
    ++it;
}

return true;  // Tip argumenata se gadja
} 

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
//=======================================================
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

// Pomocna f-ja (Provera da li je f-ja implementirana u izvedenoj klasi)
bool isImplementedInDerivedClasses(StructType* ClassType, std::string FuncName) {
    Module* ParentModule = new Module("", ClassType->getContext());

    // Iteriramo po globalnim funkcijama modula
    for (auto& F : ParentModule->functions()) {
        if (F.getName() == FuncName) {
            FunctionType* FTy = F.getFunctionType();
            if (FTy->isPointerTy()) {
                Type* FuncPointerType = FTy->getPointerElementType();
                if (StructType* DerivedType = dyn_cast<StructType>(FuncPointerType)) {
                    // Proverimo da li se izvedeni tip gadja sa tipom klase
                    if (DerivedType == ClassType) {
                        // Funkcija je implementirana u izvedenoj klasi
                        return true;
                    }
                }
            }
        }
    }

    // Funkcija nije implementirana u izvedenoj klasi
    return false;
}

// Pomocna f-ja
StructType* findStructTypeInModule(Module& M) {
    for (auto& Global : M.globals()) {
        if (StructType* STy = dyn_cast<StructType>(Global.getValueType())) {
            return STy;
        }
    }
    return nullptr;
}

// 2. Da li je virtuelna f-ja implementirana 
//=======================================================
bool hasNoImplementedVirtualFunction(Function &F) {
    // Roditeljski modul funkcije
    Module* M = F.getParent();
    if (!M) {
        // Neuspeh
        return false;
    }

    // Uzimamo tip tog roditeljskog modula kao Module* pokazivac
    Module* ParentModule = M;

    // Nama treba StructType, tu spada i klasa
    if (StructType* ClassType = dyn_cast<StructType>(findStructTypeInModule(*ParentModule))) {
        //Iteriramo po elementima klase
        for (const auto &Member : ClassType->elements()) {
            // Da li je funkcija
            if (Member->isFunctionTy()) {
                // Konvertujemo u FunctionType
                auto *FTy = dyn_cast<FunctionType>(Member);
                if (!FTy)
                    continue;

                if(isVirtual(F) && !isImplementedInDerivedClasses(ClassType,F.getName().str())){
                    return true;
                }
            }
        }
    }
    // Ako nije pronadjeno ili tip roditelja nije StructType, vracamo false
    return false;
}

// Pomocna f-ja (Da li je f-ja adekvatna )
bool isAdequateFunction(const Function& F) {
    // Proverimo da li je telo f-je prazno, tj da li ima nekih instrukcija
    if (F.empty())
        return false;

    // Provera da li f-ja ima samo jedan basic block
    // TOFIX
    if (F.size() != 1)
        return false;

    //Proveramo da li je ulazni blok funkcije prazan
    const BasicBlock& entryBlock = F.getEntryBlock();
    if (entryBlock.empty())
        return false;


    // Funkcija je adekvatna
    return true;
}

// Pomocna f-ja
// TOFIX - unused
bool isAllConstant(const ConstantDataSequential *CDS) {
  for (unsigned i = 0; i < CDS->getNumElements(); ++i) {
    Constant *Element = CDS->getElementAsConstant(i);
    if (!isa<Constant>(Element))
      return false;
  }
  return true;
}

// Pomocna f-ja 
bool isEnumType(Type *Ty) {
    if (StructType *StructTy = dyn_cast<StructType>(Ty)) {
        if (StructTy->getNumElements() == 1) {
            Type *MemberTy = StructTy->getElementType(0);

            if (MemberTy && MemberTy->isIntegerTy())
                return true;
        }
    }
    return false;
}

// 3. Provera da li je povratna vrednost f-je konstantna
//=======================================================
bool hasConstantReturnVirtualFunction(Function &F) {
    if (F.getReturnType()->isIntegerTy() || F.getReturnType()->isFloatingPointTy()) {
        if (ReturnInst *RetInst = dyn_cast<ReturnInst>(F.getEntryBlock().getTerminator())) {
            Value *RetVal = RetInst->getOperand(0);
            if (Constant *ConstRet = dyn_cast<Constant>(RetVal)) {
                if (!ConstRet->isNullValue()) {
                    return true;
                }
            }
        }
    } else if (F.getReturnType()->isPointerTy()) {
        if (ReturnInst *RetInst = dyn_cast<ReturnInst>(F.getEntryBlock().getTerminator())) {
            Value *RetVal = RetInst->getOperand(0);
            if (ConstantExpr *CE = dyn_cast<ConstantExpr>(RetVal)) {
                if (CE->getNumOperands() > 0 && CE->getOperand(0)->getType()->isAggregateType()) {
                    return true;
                }
            }
        }
    } else if (F.getReturnType()->isArrayTy()) {
        if (ReturnInst *RetInst = dyn_cast<ReturnInst>(F.getEntryBlock().getTerminator())) {
            Value *RetVal = RetInst->getOperand(0);
            if (ConstantArray *ArrayRet = dyn_cast<ConstantArray>(RetVal)) {
                if (ArrayRet->getType() && ArrayRet->getType()->isArrayTy()) {
                    // Null check for ArrayRet->getType()->getArrayNumElements()
                    if (ArrayRet->getType()->getArrayNumElements()) {
                        Constant *Element = ArrayRet->getOperand(0);
                        for (unsigned i = 1; i < ArrayRet->getNumOperands(); ++i) {
                            if (ArrayRet->getOperand(i)) {
                                if (ArrayRet->getOperand(i) != Element) {
                                    return false;
                                }
                            }
                        }
                        /*ConstantInt *ArraySize = ConstantInt::get(Type::getInt64Ty(F.getContext()), ArrayRet->getType()->getArrayNumElements());
                        if (!ArraySize) {
                            return false;
                        }*/
                    
                        return true;
                        /*Constant *Element = ArrayRet->getOperand(0);
                        for (unsigned i = 1; i < ArrayRet->getNumOperands(); ++i) {
                            if (ArrayRet->getOperand(i) != Element) {
                                return false;
                            }
                        }

                        uint64_t NumElements = ArrayRet->getType()->getArrayNumElements();
                        Type *Int64Ty = Type::getInt64Ty(F.getContext());
                        Constant *ArraySize = ConstantInt::get(Int64Ty, NumElements); */
                    }
                }
            }
        }
    } else if (isEnumType(F.getReturnType())) {
        if (ReturnInst *RetInst = dyn_cast<ReturnInst>(F.getEntryBlock().getTerminator())) {
            Value *RetVal = RetInst->getOperand(0);
            if (ConstantInt *EnumRet = dyn_cast<ConstantInt>(RetVal)) {
                for (User *U : F.users()) {
                    if (ExtractValueInst *EV = dyn_cast<ExtractValueInst>(U)) {
                        if (EV->getNumIndices() == 1 && EV->getIndices()[0] == 0) {
                            const ConstantInt *Index = dyn_cast<ConstantInt>(EV->getOperand(1));
                            if (Index && Index->getZExtValue() == 0) {
                                if (const ConstantInt *Val = dyn_cast<ConstantInt>(EV->getOperand(0))) {
                                    if (Val != EnumRet) {
                                        return false;
                                    }
                                }
                            }
                        }
                    }
                }
                return true;
            }
        }
    } else if (ReturnInst *RetInst = dyn_cast<ReturnInst>(F.getEntryBlock().getTerminator())) {
        Value *RetVal = RetInst->getOperand(0);
        if (Constant *ConstRetVal = dyn_cast<Constant>(RetVal)) {
            for (User *U : RetVal->users()) {
                if (U != RetInst) {
                    return false;
                }
            }
            return true;
        }
    } else {
        return false;
    }
}

// Pomocna f-ja (Provera da li je f-ja cista)
bool isPure(Function& F){
    // Cista virtuelna funkcija: u LLVM-u je to funkcija cija povratna vrednost iskljucivo zavisi od ulaza ili globalnih promenljivih 
    // Ako fj-a nije cista, onda funkcija ima propratne efekte i povratna vrednost ne zavisi iskljucivo od ulaza
    
    // Iteriramo po basic blokovima
    for(BasicBlock &bb : F){
        for(Instruction &i : bb){
            if(i.mayHaveSideEffects()){
                return false;
            }
        }
    }
    return true;
}

// 4. Provera da li je f-ja koriscena 
//=======================================================
bool isUnusedVirtualFunction(Function &F) {
    
    // Provera da li je F virtuelna
    if (!isVirtual(F)) {
        return false;
    }

    // Ovo znaci da li je funkciji moguce pristupiti iz drugih jedinica prevodjenja(drugi modul npr). Mislim da moze biti korisno 
    if (F.hasExternalLinkage()) {
        return false;
    }

    // Da li je funkcija overriden-ovana od strane bilo koje druge funkcije
    if (F.isOneValue()) {
        return false;
    }

    // Da li se funkcija poziva iz bilo koje druge funkcije
    if (F.getNumUses() > 0) {
        return false;
    }

    // Da li je i definisana u modulu, ne samo deklarisana
    if (!F.isDeclaration()) {
        return false;
    }

   
    // Ako uslovi iznad nisu ispunjeni, racunamo da je nekoriscena(verovatno ima jos nekih uslova, ali ne mogu da se setim, ni da pronadjem na netu uslove)
    return true;
  }

};   // kraj namespace-a 

char DeadVirtualFunctionEliminationPass::ID = 0;
static RegisterPass<DeadVirtualFunctionEliminationPass> X("DVFE",
                                                         "Dead Virtual Function Elimination Pass",
                                                         false, false);
}