import sys
import subprocess
import os
import time
from subprocess import Popen, PIPE
subprocess.getoutput("rm -r -f out")
subprocess.getoutput("mkdir out")
true_outputs = [
1, #test 1
1, #test 2
1, #test 3
1, #test 4
1, #test 5
1, #test 6
1, #test 7
0, #test 8
0, #test 9
1  #test 10 
]
for i in range(1,len(true_outputs)+1):
    print("---------TEST {0}----------".format(i))
    pipe = Popen(["./ekcc", "-emit-ast", "-o", 
                  "./out/test{0}.yaml".format(i), 
                  "./tests/test{0}.ek".format(i)], 
                  stdout=PIPE)
    test_output = pipe.communicate()[0].decode("utf-8")
    print(test_output)
    print("test_rv: {0}, expected_rv:{1}".format(pipe.returncode, true_outputs[i-1]) )
    if (true_outputs[i-1]==0 and pipe.returncode!=0) or (true_outputs[i-1]!=0 and pipe.returncode==0):
        print("Test {0} failed".format(i))
    else:
        print("Test {0} pass".format(i))