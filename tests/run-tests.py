import sys
import subprocess
import os
import time

from subprocess import Popen, PIPE


subprocess.getoutput("rm -r -f out")
subprocess.getoutput("mkdir out")
true_outputs = [1, 1, 1, 1, 1, 1, 1, 0, 0]
for i in range(len(true_outputs)):
    print("---------TEST {0}----------".format(i))
    pipe = Popen(["./ekcc", "-emit-ast", "-o", 
                  "./out/test{0}.yaml".format(i), 
                  "./tests/test{0}.ek".format(i)], 
                  stdout=PIPE)
    test_output = pipe.communicate()[0].decode("utf-8")
    print(test_output)

#     test_output = subprocess.getoutput("./ekcc -emit-ast -o ./out/test{0}.yaml ./tests/test{0}.ek".format(i))
#     print(test_output)
    if pipe.returncode != true_outputs[i]:
        print("Test {0} fail: Test return value: {1}, Expected return value: {2}"
              .format(i, pipe.returncode, true_outputs[i]))
    else:
        print("Test {0} pass".format(i))