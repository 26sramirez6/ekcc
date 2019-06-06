Compiler for Extended-Kaleidoscope Language
-
To produce the ekcc executable, 
1. set LLVM_ROOT in the Makefile to the root location of llvm version 7 installation
2. run "make"

Please note input-file has to end with .ek.
-
-h | -?		For help/useful message.
-v		Verbose mode
-O		Add optimizations
-emit-ast	To output AST in .yaml format
-emit-llvm	Output llvm
-o 		Output <output-file> <input-file>
-jit		run in jit mode

We have included our own test-suite that runs through several sample input 
files located in "tests/" and checks the return codes and error message print outs.

To run the test suite “make tests”.
The tests are located in the ./tests folder
Test files “test<1-7>.ek” are are tests for syntax errors
Test files “test<8-9>.ek” are long tests for valid syntax that should produce no error
test files “test<10-12>.ek” are harder tests for syntax errors
test files “test<13-29>.ek” are easy tests for code generation
test files “test<30-36>.ek” are harder tests for arg/argf/print code generation

The full test suite is ran twice, once with no optimization flag and again with optimization flags.
