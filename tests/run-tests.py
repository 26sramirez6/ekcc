import sys
import subprocess
import os
import time
from subprocess import Popen, PIPE
subprocess.getoutput("rm -r -f out")
subprocess.getoutput("mkdir out")
true_outputs = [
# ast tests below
1, #test 1
1, #test 2
1, #test 3
1, #test 4
1, #test 5
1, #test 6
1, #test 7
0, #test 8
0, #test 9
1, #test 10 
1, #test 11
1, #test 12

# llvm basic tests
(0,4), #test 13
(0,9), #test 14
(0,18),#test 15
(0,3), #test 16
(0,8), #test 17
(0,32), #test 18
(0,10), #test 19
(0,7), #test 20
(0,7), #test 21
(0,10), #test 22
(0,13), #test 23
(0,19), #test 24
(0,25), #test 25
(0,10), #test 26

# print tests
(0,5,"5\n"), #test 27 
(0,5,"5\n"), #test 28
(0,0,"8.500000\n"), #test 29
(0,0,"hello world\n"), #test 30

# arg/argf tests
(0, 5, "5"), #test 31
(0, 0, "1.2"), #test 32

# cint over flow tests
(0, 1), # test 33
(0, 1), # test 34
(0, 1), # test 35
(0, 1), # test 36
(0, 1), # test 37
]
total_passed = 0
failed = []
# first pass is non-optimized, second pass is optimized
for j in range(2):
    for i in range(1,len(true_outputs)+1):
        testNum = j*len(true_outputs) + i
        print("---------TEST {0}----------".format(testNum))
        if i < 13:
            continue
            pipe = Popen(["./ekcc", "-o", "./tests/out/test{0}.yaml".format(i),
                        "./tests/test{0}.ek".format(i)], 
                        stdout=PIPE)
                        
            test_output = pipe.communicate()[0].decode("utf-8")
            print(test_output)
        
            print("test_rv: {0}, expected_rv:{1}".format(pipe.returncode, true_outputs[i-1]) )
        
            if (true_outputs[i-1]==0 and pipe.returncode!=0) or (true_outputs[i-1]!=0 and pipe.returncode==0):
                print("Test {0} failed".format(testNum))
                failed.append(i)
            else:
                print("Test {0} pass".format(testNum))
                total_passed += 1
        else:
            if j==0:
                exename = "./out/test{0}".format(i)
                pipe = Popen(["./ekcc", "-o", 
                            exename, 
                            "./tests/test{0}.ek".format(i)], 
                            stdout=PIPE)
            else:
                exename = "./out/test{0}opt".format(i)
                pipe = Popen(["./ekcc", "-O", "-o", 
                            exename, 
                            "./tests/test{0}.ek".format(i)], 
                            stdout=PIPE)
                        
            test_output = pipe.communicate()[0].decode("utf-8")
            print(test_output)
            print("test_rv: {0}, expected_rv:{1}".format(pipe.returncode, true_outputs[i-1][0]) )
        
            if (true_outputs[i-1][0]==0 and pipe.returncode!=0) or (true_outputs[i-1][0]!=0 and pipe.returncode==0):
                print("Test {0} failed".format(testNum))
                failed.append(i)
            else:
                # now run it
                if os.path.isfile(exename):
                    if i==31 or i==32:
                        pipe = Popen([exename, 
                                    true_outputs[i-1][2]],
                                    stdout=PIPE)
                    else:
                        pipe = Popen([exename], stdout=PIPE)
                    test_output = pipe.communicate()[0].decode("utf-8")
                    if 26<i<31: 
                        print(test_output)
                        if test_output!=true_outputs[i-1][2]:
                            print("Test {0} failed".format(testNum))
                            failed.append(i)
                            continue
                    print("test_rv: {0}, expected_rv:{1}".format(pipe.returncode, true_outputs[i-1][1]) )
                    if pipe.returncode!=true_outputs[i-1][1]:
                        print("Test {0} failed".format(testNum))
                        failed.append(i)
                    else:
                        total_passed += 1
                        print("Test {0} pass".format(testNum))
                else:
                    print("Test {0} failed".format(testNum))
                    failed.append(i)
        
print("{0}/{1} tests passed".format(total_passed, len(true_outputs)*2))

if len(failed):
    print("test failures: {0}".format(failed))