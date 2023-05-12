
RED='\033[0;31m' # Red Color
NC='\033[0m' # No Color

if [[ $1 != *".cpp"* ]]; then
    # Nothing to see here, onwards! :d
    if [[ $1 != *".ll"* ]] ; then
        echo -e "${RED}Test example is of wrong extension!${NC}"
        exit 
    fi
fi

echo -e "${RED} Do you want to recompile this pass?[y/n]${NC}"
read answer
if [ "$answer" = "y" ]; then
    cd ../../../../build/lib
    make
fi 

if [[ $1 == *".cpp"* ]]; then
    cd TestFiles 
    clang -S -O0 -emit-llvm $1 
fi 

cd ../../../../build/lib
echo -e "${RED} Loading our pass...${NC}"
./../bin/opt -load LLVMDeadVirtual.so -enable-new-pm=0 -DVFE ./../../llvm/lib/Transforms/llvm-dead-virtual-pass/TestFiles/$1 -S > ../../llvm/lib/Transforms/llvm-dead-virtual-pass/TestFiles/PassIrOutput.s