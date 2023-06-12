# llvm-dead-virtual-function-elimination-pass



## Guide to using the script 
    -> If you are using the script for the first time, give it permission to run via chmod
    -> The script should not be moved to other folders since it's been made to work only from path 

    usage: ./testScript.sh (testExampleName).(ll || cpp)
        a) If you want to recompile your pass, you can do so via script
        b) Script will also auto run a test on the testExample given in the argument of the script
        c) Generated IR output of the pass will end up in TestFiles folder with the name "PassIROutput.s"

## Guide to writing tests
    -> Our pass detects virtual functions by name. It looks for either  _virtual sufix or virtual_ prefix in the name
    -> Once you write the test pass you should call it with testScript (guide is above)
