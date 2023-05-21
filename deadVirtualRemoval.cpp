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
#include <vector>

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
  
  bool runOnModule(Module &M) override {
    CG = &getAnalysis<CallGraphWrapperPass>().getCallGraph();
    bool Changed = false;  // Provera: ukoliko se bude transformisao kod, tj. ukoliko se eliminise mrtva virtuelna funkcija, postace true

    for (Function &F : M) {
      if (!F.isDeclaration() && !F.isIntrinsic() && F.getName().find("_Z") != 0) {
        dbgs() << F.getName() << "   5 \n";
        // Za svaku funkciju iz modula proveravamo da li zadovoljava uslov neke od ovih funkcija. Ako da, brisemo je, jer ukoliko zadovoljava bilo koju od ovih funkcija, eliminacija ne remeti rad modula
          if(hasNoImplementedVirtualFunction(F) || hasConstantReturnVirtualFunction(F)) {
              if(isAdequateFunction(F)) {
                dbgs() << F.getName() << "\n";
                F.deleteBody();
                Changed = true;
              }
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
  bool hasMatchingArgumentTypes(const Function &F, const std::vector<Type*> &expectedTypes) {
    if (F.arg_size() != expectedTypes.size()) {
      return false;  // Different number of arguments
    }

    auto it = expectedTypes.begin();
    for (const Argument &Arg : F.args()) {
      if (Arg.getType() != *it) {
        return false;  // Argument type mismatch
      }
      ++it;
    }

    return true;  // All argument types match
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

bool isImplementedInDerivedClasses(StructType* ClassType, std::string FuncName) {
    Module* ParentModule = new Module("", ClassType->getContext());

    // Iterate over the global functions in the module
    for (auto& F : ParentModule->functions()) {
        if (F.getName() == FuncName) {
            FunctionType* FTy = F.getFunctionType();
            if (FTy->isPointerTy()) {
                Type* FuncPointerType = FTy->getPointerElementType();
                if (StructType* DerivedType = dyn_cast<StructType>(FuncPointerType)) {
                    // Check if the derived type matches the class type
                    if (DerivedType == ClassType) {
                        // Function is implemented in the derived class
                        return true;
                    }
                }
            }
        }
    }

    // Function is not implemented in any derived classes
    return false;
}

StructType* findStructTypeInModule(Module& M) {
    for (auto& Global : M.globals()) {
        if (StructType* STy = dyn_cast<StructType>(Global.getValueType())) {
            return STy;
        }
    }
    return nullptr;
}

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
        for (const auto &Member : ClassType->elements()) {
            // Da li je funkcija
            if (Member->isFunctionTy()) {
                // Konvertujemo u FunctionType
                auto *FTy = dyn_cast<FunctionType>(Member);
                if (!FTy)
                    continue;

                // Da li funkcija ima vtable index
                if (FTy->getNumParams() > 0 && FTy->getParamType(0) == F.getType()) {
                    // Da li je funkcija virtuelna
                    if (F.hasFnAttribute("vtable-index")) {
                        // Jeste virtuelna
                        // Dohvatamo ime funkcije
                        std::string FuncName = F.getName().str();

                        // Proveravamo da li funkciju implementira neka od izvedenih klasa 
                        if (!isImplementedInDerivedClasses(ClassType, FuncName)) {
                            return true;
                        }
                    }
                }
            }
        }
    }

    // Ako nije pronadjeno ili tip roditelja nije StructType, vracamo false
    return false;
}

bool isAdequateFunction(const Function& F) {
    // Check if the function is empty
    if (F.empty())
        return false;

    // // Check if the function has a single basic block
    // if (F.size() != 1)
    //     return false;

    //Check if the function's entry block is empty
    const BasicBlock& entryBlock = F.getEntryBlock();
    if (entryBlock.empty())
        return false;

    // // Check if the function has any instructions
    // for (const Instruction& I : F.front()) {
    //     // Add your conditions here to check for adequate instructions
    //     // For example, check if the instruction is a return statement
    //     if (const ReturnInst* RI = dyn_cast<ReturnInst>(&I)) {
    //         // Add your condition to check if the return statement is adequate
    //         // For example, check if the return value is a constant integer 42
    //         if (ConstantInt* CI = dyn_cast<ConstantInt>(RI->getReturnValue())) {
    //             if (CI->getZExtValue() == 42)
    //                 return true;
    //         }
    //     }
    // }

    // Function is adequate
    return true;
}

bool isAllConstant(const ConstantDataSequential *CDS) {
  for (unsigned i = 0; i < CDS->getNumElements(); ++i) {
    Constant *Element = CDS->getElementAsConstant(i);
    if (!isa<Constant>(Element))
      return false;
  }
  return true;
}

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


  /*
  bool hasConstantReturnVirtualFunction(Function &F) {
    
    // Proveravamo da li funkcija ima konstantnu povratnu vrednost koja je numericka (int ili floating point)
    if (F.getReturnType()->isIntegerTy() || F.getReturnType()->isFloatingPointTy()) {
        Constant *ConstRet = dyn_cast<Constant>(F.getReturnValue());
        if (ConstRet && !ConstRet->isNullValue()) {
            return true;
        }
    }
    // Proveravamo da li funkcija ima konstantnu povratnu vrednost koja je string sa konstantim vrednostima(to radi ovaj poslednji ugnjezdeni if)
    else if (F.getReturnType()->isPointerTy()) {
        if (ConstantExpr *CE = dyn_cast<ConstantExpr>(F.getReturnValue())) {
            if (CE->isGEPWithNoNotionalOverIndexing()) {
                if (ConstantDataSequential *CDS = dyn_cast<ConstantDataSequential>(CE->getOperand(0))) {
                    if (CDS->isString() && CDS->isAllConstant()) {
                        return true;
                    }
                }
            }
        }
    } 
    // Proveravamo da li funkcija ima konstantnu povratnu vrednost koja je niz sa konstantim vrednostima(ovo za konstantnu duzinu je mozda redundantno, ali neka stoji za sada)
    else if (F.getReturnType()->isArrayTy()) {
        ConstantArray *ArrayRet = dyn_cast<ConstantArray>(F.getReturnValue());
        if (ArrayRet) {
            // Proveravamo da li svi elementi niza imaju konstantne vrednosti
            Constant *Element = ArrayRet->getOperand(0);
            for (unsigned i = 1; i < ArrayRet->getNumOperands(); ++i) {
                if (ArrayRet->getOperand(i) != Element) {
                    return false;
                }
            }
            // Sada proveravamo da li je duzina niza konstantna
            ConstantInt *ArraySize = dyn_cast<ConstantInt>(ArrayRet->getType()->getArrayNumElements());
                if (!ArraySize) {
                    return false;
                }
            return true;
        }
    }
    // Proveravamo da li funkcija ima konstantnu povratnu vrednost koja je enum
    else if (F.getReturnType()->isEnumTy()) {
        ConstantInt *EnumRet = dyn_cast<ConstantInt>(F.getReturnValue());
        if (EnumRet) {
            // Provera da li je enum vrednost konstantna u svim slucajevima (i ovo sam iskopao negde, moracu da pojasnim)
            for (const User *U : F.users()) {
                if (const ExtractValueInst *EV = dyn_cast<ExtractValueInst>(U)) {
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
    // Proveravamo da li funkcija ima konstantnu povratnu vrednost koja je kontantan izraz(vrednost poznata u fazi kompilacije)
    else if (F.getReturnValue()->isConstantExpr()) {
        ConstantExpr *ExprRet = dyn_cast<ConstantExpr>(F.getReturnValue());
        if (ExprRet) {
            // Provera da li je konstantan izraz uvek iste vrednost
            Constant *Result = ExprRet->getWithOperandReplaced(0, F.getReturnValue());
            for (unsigned i = 1; i < ExprRet->getNumOperands(); ++i) {
                Constant *Op = ExprRet->getOperand(i);
                if (Op != Result) {
                    return false;
                }
            }
            return true;
        }
    }


    // Proveravamo da li funkcija u svim svojim implementacijama vraca jednu te istu vrednost
    for (auto *U : F.users()) {
        if (auto *Call = dyn_cast<CallInst>(U)) {
            Function *Caller = Call->getFunction();
            if (Caller && Caller->isDeclaration()) {
                // Caller je deklaracija, pa time nije implementacija F
                continue;
            }

            Value *RetVal = Call->getArgOperand(0);
            if (RetVal && RetVal->getType()->isIntegerTy() && RetVal->hasOneUse()) {
                if (auto *CI = dyn_cast<ConstantInt>(RetVal)) {
                // Proveravamo da li je konstanta povratna vrednost ista u svim implementacijama 
                    for (User *U2 : CI->users()) {
                        if (auto *CI2 = dyn_cast<ConstantInt>(U2)) {
                            if (CI2->getSExtValue() != CI->getSExtValue()) {
                                return false;
                            }
                        } else {
                            return false;
                        }
                    }

                    return true;
                }
            }
        }
    }

    return false;
  }

  */

};   // kraj namespace-a 

char DeadVirtualFunctionEliminationPass::ID = 0;
static RegisterPass<DeadVirtualFunctionEliminationPass> X("DVFE",
                                                         "Dead Virtual Function Elimination Pass",
                                                         false, false);
}
