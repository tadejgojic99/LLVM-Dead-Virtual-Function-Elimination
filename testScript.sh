
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
    cd ../../../../build
    make
    cd ./../llvm/lib/Transforms/llvm-dead-virtual-pass/
fi

fName=$1
if [[ $fName == *".cpp"* ]]; then
    cd TestFiles 
    clang -S -O0 -emit-llvm $1 
    fName="${fName//.cpp/.ll}"
    cd ./../
else 
    cd TestFiles
    rm *.s
    cd ./../
fi 

cd ./../../../../build/lib/
echo -e "${RED} Loading our pass...${NC}"
./../bin/opt -load LLVMDeadVirtual.so -enable-new-pm=0 -DVFE ./../../llvm/lib/Transforms/llvm-dead-virtual-pass/TestFiles/$fName -S > ../../llvm/lib/Transforms/llvm-dead-virtual-pass/TestFiles/PassIrOutput.s