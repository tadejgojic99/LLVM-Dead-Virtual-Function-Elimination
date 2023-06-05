#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/PassInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
//----------------------------------------
#include <regex>
#include <vector>

using namespace llvm;

namespace {

// Definisemo pass: buduci da je u pitanju eliminacija mrtvih virtuelnih
// funkcija, nasledjujemo ModulePass
class DeadVirtualFunctionEliminationPass : public ModulePass {
public:
  static char ID;
  CallGraph *CG;
  DeadVirtualFunctionEliminationPass() : ModulePass(ID) {}

  bool runOnModule(Module &M) override {
    CG = &getAnalysis<CallGraphWrapperPass>().getCallGraph();
    bool Changed =
        false; // Provera: ukoliko se bude transformisao kod, tj. ukoliko se
               // eliminise mrtva virtuelna funkcija, postace true

    std::vector<Function *> markedForDelete;
    for (Function &F : M) {
      if (!F.isDeclaration() && !F.isIntrinsic() && isVirtual(F)) {
        // Za svaku funkciju iz modula proveravamo da li zadovoljava uslov neke
        // od ovih funkcija. Ako da, brisemo je, jer ukoliko zadovoljava bilo
        // koju od ovih funkcija, eliminacija ne remeti rad modula
        /*
        bool isAdequate = isAdequateFunction(F);
        bool isEmpty = isEmptyVirtualFunction(F);
        bool isUnreachable = isUnreachableVirtualFunction(F);
        bool hasNoImplemented = hasNoImplementedVirtualFunction(F);
        bool hasConstant = hasConstantReturnVirtualFunction(F);
        */

        if (isEmptyVirtualFunction(F) || isUnreachableVirtualFunction(F, M) ||
            // hasNoImplementedVirtualFunction(F) ||
            hasConstantReturnVirtualFunction(F)) {

          deleteDeadCallsitesInMain(F, M);
          markedForDelete.push_back(&F);

          Changed = true;
        }
      }
    }
    deleteMarkedFunctions(markedForDelete);

    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const {
    // Oznacavamo da se analiza CallGraphWrapperPass izvrsi pre pass-a
    AU.addRequired<CallGraphWrapperPass>();
  }

private:
  // Ova funkcija brise instrukcije koje pozivaju mrtve f-je u funkciji main
  void deleteDeadCallsitesInMain(Function &F, Module &M) {
    BasicBlock &BB = *M.getFunction("main")->begin();
    for (BasicBlock::iterator itINST = BB.begin(); itINST != BB.end();
         itINST++) {
      Instruction &INST = *itINST;
      if (const CallInst *CInst = dyn_cast<CallInst>(&INST)) {
        if (CInst->getCalledOperand()->getName() == F.getName()) {
          INST.replaceAllUsesWith(UndefValue::get(INST.getType()));
          INST.eraseFromParent();
          break;
        }
      }
    }
  }

  // Brise funkcije koje su oznacene kao mrtve
  void deleteMarkedFunctions(std::vector<Function *> &markedFunctions) {
    if (markedFunctions.empty())
      return;

    for (auto func : markedFunctions) {
      dbgs() << "Deleting " << func->getName() << "..."
             << "\n";
      func->replaceAllUsesWith(UndefValue::get(func->getType()));
      func->eraseFromParent();
    }
  }

  // Pomocna f-ja
  // Proveramo da li je f-ja virtuelna na osnovu (virtual_) tj svaka f-ja u svom
  // nazivu mora imati virtual_ Primer virtual_<ime_fje> ili <ime_fje>_virtual
  bool isVirtual(Function &F) {
    return std::regex_match(F.getName().str(), std::regex("(.*)virtual_(.+)"))
               ? true
               : false;
  }

  int countInstructionsInFunction(Function &F) {
    unsigned instructionCount = 0;
    for (const BasicBlock &BB : F) {
      for (const Instruction &I : BB) {
        instructionCount++;
      }
    }

    return instructionCount;
    // dbgs() << "Function name: " << F.getName() << ", Instruction Count: " <<
    // instructionCount << "\n";
  }

  // Ovde definisemo te funkcije koje su nam kriterijumi za identifikaciju
  // mrtvih virtuelnih funkcija
  bool hasMatchingArgumentTypes(const Function &F,
                                const std::vector<Type *> &expectedTypes) {
    if (F.arg_size() != expectedTypes.size()) {
      return false; // Razlicit broj argumenata
    }

    auto it = expectedTypes.begin();
    for (const Argument &Arg : F.args()) {
      if (Arg.getType() != *it) {
        return false; // Razlicit tip argumenata
      }
      ++it;
    }

    return true; // Tip argumenata se gadja
  }

  bool isCalledFromFunction(const Function *F, const Function *Caller) {
    // Prica sa vezbi BB -> Instructions, iteriramo kroz instrukcije
    // instrukkcije Caller funkcije
    for (const BasicBlock &BB : *Caller) { // *Caller
      for (const Instruction &Inst : BB) {
        // Proveravamo da li je instrukcija 'call'
        if (const CallInst *CI = dyn_cast<CallInst>(&Inst)) {
          // Uzimamo pozvanu funkciju iz instrukcije
          const Function *CalledFunc = CI->getCalledFunction();

          if (CalledFunc && CalledFunc->getName() == F->getName()) {
            return true;
          }
        }
      }
    }

    return false;
  }

  // 1. Nedostizne virtuelne funkcije
  // Trazimo pozvane funkcije u nasoj funkciji i pokusavamo da vidim
  // da li postoji grana poziva iz nje ka njima
  //=======================================================
  bool isUnreachableVirtualFunction(const Function &F, Module &M) {
    CallGraphNode *CGNode = (*CG)[&F];

    BasicBlock &BB = *M.getFunction("main")->begin();
    for (BasicBlock::iterator itINST = BB.begin(); itINST != BB.end();
         itINST++) {
      Instruction &INST = *itINST;
      if (const CallInst *CInst = dyn_cast<CallInst>(&INST)) {
        const Function *Callee = CInst->getCalledFunction();

        if (Callee->getName() == F.getName())
          return false;

        if (isCalledFromFunction(&F, Callee)) {
          // Ako nemamo granu poziva iz trenutne funkcije F u pozvanu funkciju
          // Callee, onda je virtuelna funkcija nedostizna
          return false;
        }
      }
    }
    // Ako su sve grane poziva iz trenutne funkcije prisutne u pozvanim
    // funkcijama, onda je virtuelna funkcija dostizna
    dbgs() << F.getName() << " -> should be deleted "
           << "detected in *isUnreachableVirtualFunction* "
           << "\n";
    return true;
  }

  // Pomocna f-ja (Provera da li je f-ja implementirana u izvedenoj klasi)
  bool isImplementedInDerivedClasses(StructType *ClassType,
                                     std::string FuncName) {
    Module *ParentModule = new Module("", ClassType->getContext());

    // Iteriramo po globalnim funkcijama modula
    for (auto &F : ParentModule->functions()) {
      // Trazimo f-ju sa istim imenom kao ulazna f-ja
      if (F.getName() == FuncName) {
        FunctionType *FTy = F.getFunctionType();
        if (FTy->isPointerTy()) {
          Type *FuncPointerType = FTy->getPointerElementType();
          if (StructType *DerivedType = dyn_cast<StructType>(FuncPointerType)) {
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
  // Trazi Klasu u Modulu
  StructType *findStructTypeInModule(Module &M) {
    for (auto &Global : M.globals()) {
      if (StructType *STy = dyn_cast<StructType>(Global.getValueType())) {
        return STy;
      }
    }
    return nullptr;
  }

  // 2. Da li je virtuelna f-ja implementirana
  //=======================================================
  bool hasNoImplementedVirtualFunction(Function &F) {
    // Roditeljski modul funkcije
    Module *M = F.getParent();
    if (!M) {
      // Neuspeh
      return false;
    }

    // Uzimamo tip tog roditeljskog modula kao Module* pokazivac
    Module *ParentModule = M;

    // Nama treba StructType, tu spada i klasa
    if (StructType *ClassType =
            dyn_cast<StructType>(findStructTypeInModule(*ParentModule))) {
      // Iteriramo po elementima klase
      for (const auto &Member : ClassType->elements()) {
        // Da li je funkcija
        if (Member->isFunctionTy()) {
          // Konvertujemo u FunctionType
          auto *FTy = dyn_cast<FunctionType>(Member);
          if (!FTy)
            continue;

          if (isVirtual(F) &&
              !isImplementedInDerivedClasses(ClassType, F.getName().str())) {
            return true;
          }
        }
      }
    }
    // Ako nije pronadjeno ili tip roditelja nije StructType, vracamo false
    return false;
  }

  // Pomocna f-ja (Da li je f-ja adekvatna )
  bool isAdequateFunction(const Function &F) {
    // Proverimo da li je telo f-je prazno, tj da li ima nekih instrukcija
    if (F.empty())
      return false;

    // Provera da li f-ja ima jedan ili vise basic blokova
    if (F.size() < 1)
      return false;

    // Proveramo da li je ulazni blok funkcije prazan
    const BasicBlock &entryBlock = F.getEntryBlock();
    if (entryBlock.empty())
      return false;

    // Funkcija je adekvatna
    return true;
  }

  // Pomocna f-ja koja za proverava da li su svi elementi konstantne
  bool isAllConstant(const ConstantDataSequential *CDS) {
    for (unsigned i = 0; i < CDS->getNumElements(); ++i) {
      Constant *Element = CDS->getElementAsConstant(i);
      if (!isa<Constant>(Element))
        return false;
    }
    return true;
  }

  // Pomocna f-ja
  // bool isEnumType(Type *Ty) {
  //   dbgs() << "Tip: " << Ty << "\n";
  //   if (StructType *StructTy = dyn_cast<StructType>(Ty)) {
  //     if (StructTy->getNumElements() >= 1) {
  //       Type *MemberTy = StructTy->getElementType(0);

  //       if (MemberTy && MemberTy->isIntegerTy())
  //         return true;
  //     }
  //   }
  //   return false;
  // }

  // 3. Provera da li je povratna vrednost f-je konstantna
  //=======================================================
  bool hasConstantReturnVirtualFunction(Function &F) {
    // dbgs() << F.getName() << "\n";
    //  Prolazimo kroz sve Basic Block-ove i njihove instrukcije, kako bi
    //  nasli return
    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        if (ReturnInst *RetInst = dyn_cast<ReturnInst>(&I)) {
          Value *RetVal = RetInst->getOperand(0);
          if (Constant *ConstRet = dyn_cast<Constant>(RetVal)) {
            // Da li je konstanta tipa niza
            if (ArrayType *ArrTy = dyn_cast<ArrayType>(ConstRet->getType())) {
              if (ArrTy->isSized()) {
                dbgs() << F.getName() << " -> should be deleted "
                       << "detected in *hasConstantReturnVirtualFunction* "
                       << "\n";
                return true;
              }
            }
            // Pronasao da u LLVM-u konvertuje enum u int, nema zasebnu
            // podrsku za enum if (isEnumType(F.getReturnType())) {
            //   // kastujemo u ConstantInt
            //   if (ConstantInt *EnumRet = dyn_cast<ConstantInt>(RetVal)) {
            //     for (User *U : F.users()) {
            //       // Uzimamo instrukciju i proveravamo da li je svaki put
            //       ista vrednost enum-a, u suprotnom bi samo proveravalo tip
            //       if (ExtractValueInst *EV = dyn_cast<ExtractValueInst>(U))
            //       {
            //         if (EV->getNumIndices() == 1 && EV->getIndices()[0] ==
            //         0) {
            //           const ConstantInt *Index =
            //               dyn_cast<ConstantInt>(EV->getOperand(1));
            //           if (Index && Index->getZExtValue() == 0) {
            //             // Proveravamo da li se poklapa ta vrednost
            //             if (const ConstantInt *Val =
            //                     dyn_cast<ConstantInt>(EV->getOperand(0))) {
            //               if (Val != EnumRet) {
            //                 return false;
            //               }
            //             }
            //           }
            //         }
            //       }
            //     }
            //   }
            //   return true;
            // }

            // Pointer tipovi
            else if (F.getReturnType()->isPointerTy()) {
              dbgs() << F.getName() << " -> should be deleted "
                     << "detected in *hasConstantReturnVirtualFunction* "
                     << "\n";
              return true;

            }
            // Tip strukture
            else if (F.getReturnType()->isAggregateType()) {
              dbgs() << F.getName() << " -> should be deleted "
                     << "detected in *hasConstantReturnVirtualFunction* "
                     << "\n";
              return true;
            }
            // Enum tip, char, integer i float
            // Provera za konstantne povratne vrednosti
            // Zavisno od tipa povratne vrednosti funkcije
            // Integer ili floating-point tipovi
            else if (F.getReturnType()->isIntegerTy() ||
                     F.getReturnType()->isFloatingPointTy()) {
              if (!ConstRet->isNullValue()) {
                dbgs() << F.getName() << " -> should be deleted "
                       << "detected in *hasConstantReturnVirtualFunction* "
                       << "\n";
                return true;
              }
            }
          }
        }
      }
    }

    return false; // Nije nadjena ReturnInst u funkciji
  }

  // 4. Provera da li je f-ja prazna
  //=======================================================
  bool isEmptyVirtualFunction(Function &F) {

    int counter = 0;
    for (BasicBlock &bb : F) {
      // Iteriramo po instrukcijama i pitamo da li neka instrukcija ima
      // propratni efekat
      for (Instruction &i : bb) {
        counter++;
      }
    }

    // Ovo se odnosi na void metode funkcije
    // Svaka klasna metoda ima bar cetiri instrukcije
    // 3 instrukcije koje se odnose na klasni pokazivac this
    // 1 return instrukcija
    // Ako ima <= 4 instrukcije to znaci da je ona prazna
    if (counter <= 4 && F.getReturnType()->isVoidTy()) {
      dbgs() << F.getName() << " -> should be deleted "
             << "detected in *isEmptyVirtualFunction* "
             << "\n";

      return true;
    }

    // Ako gornji uslov nije ispunjen onda funkcija nije prazna
    return false;
  }

}; // kraj namespace-a

char DeadVirtualFunctionEliminationPass::ID = 0;
static RegisterPass<DeadVirtualFunctionEliminationPass>
    X("DVFE", "Dead Virtual Function Elimination Pass", false, false);
} // namespace