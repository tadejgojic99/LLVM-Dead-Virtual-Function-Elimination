# llvm-dead-virtual-pass



## Guide to using the script 
    -> If you are using the script for the first time give it permission to run via chmod
    -> The script should not be moved to other folders since its made to work only from path 

    usage: ./testScript.sh (testExampleName).(ll || cpp)
        a) If you want recompile your pass you can do so via script
        b) Script will auto run test on the testExample given in the argument of the script
        c) Generated Ir output of the pass will end up in TestFiles folder with the name "PassIrOutput.s"