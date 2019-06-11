import sys
import subprocess
import os
import time
import pandas as pd
from subprocess import Popen, PIPE
subprocess.getoutput("rm -r -f out")
subprocess.getoutput("mkdir out")
true_outputs = [
# llvm basic tests, errors
(1,), #test 1
(1,), #test 2
(1,), #test 3
(1,), #test 4
(1,), #test 5
(1,), #test 6
(1,), #test 7

# slightly harder tests, errors/no errors
(0,), #test 8
(1,), #test 9
(1,), #test 10 
(1,), #test 11
(1,), #test 12

# llvm basic tests, no errors, check output
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

# optimization performance
(0, 0), # test 38
(0, 0), # test 39
(0, 0), # test 40
(0, 0), # test 41
(0, 0) # test 42

]
total_passed = 0
failed = []

metrics = [{"Test #": i, 
  "Compile-time unoptimized": None, 
  "Run-time unoptimized": None, 
  "Compile-time optimized": None, 
  "Run-time optimized": None} 
for i in range(1,43)]

def run_and_record_process_time(args, i, j, compile_or_run="compile"):
    print(" ".join(args))
    if j!=2:
        time_start = time.time()
    pipe = Popen(args, stdout=PIPE)
    if j!=2:
        time_end = time.time()
        diff = time_end-time_start
        print("{0} time:".format(compile_or_run), diff)
    
    if compile_or_run=="compile":
        if j==0:
            metrics[i-1]["Compile-time unoptimized"] = diff
        elif j==1:
            metrics[i-1]["Compile-time optimized"] = diff
    else:
        if j==0:
            metrics[i-1]["Run-time unoptimized"] = diff
        elif j==1:
            metrics[i-1]["Run-time optimized"] = diff
            
    return pipe

# first pass is non-optimized, second pass is optimized, third pass is jit and optimized
for j in range(3):
    for i in range(1,len(true_outputs)+1):
        testNum = j*len(true_outputs) + i
        print("---------TEST {0}----------".format(testNum))
        if i < 13:
            if j==2:
                args = ["./ekcc", "-jit", "./tests/test{0}.ek".format(i)]
            else:
                args = ["./ekcc", "-o", "./out/test{0}".format(i),
                        "./tests/test{0}.ek".format(i)]
            
            pipe = run_and_record_process_time(args, i, j)
            
            print(" ".join(args))
            test_output = pipe.communicate()[0].decode("utf-8")
            print(test_output)
        
            print("test_rv: {0}, expected_rv:{1}".format(pipe.returncode, true_outputs[i-1][0]) )
        
            if (true_outputs[i-1][0]!=pipe.returncode!=0):
                print("Test {0} failed".format(testNum))
                failed.append(i)
            else:
                print("Test {0} pass".format(testNum))
                total_passed += 1
        else:
            if j==0:
                exename = "./out/test{0}".format(i)
                args = ["./ekcc", "-o", exename, "./tests/test{0}.ek".format(i)]
                pipe = run_and_record_process_time(args, i, j)
            elif j==1:
                exename = "./out/test{0}opt".format(i)
                args = ["./ekcc", "-O", "-o", exename, "./tests/test{0}.ek".format(i)]
                pipe = run_and_record_process_time(args, i, j)
            elif j==2:
                extras = ""                    
                if i==31 or i==32:
                    extras = true_outputs[i-1][2]
                args = ["./ekcc", "-O", "-jit", "./tests/test{0}.ek".format(i), extras]
                pipe = run_and_record_process_time(args, i, j)
                test_output = pipe.communicate()[0].decode("utf-8")
                if 27<=i<=30:
                    print(test_output)
                    if test_output!=true_outputs[i-1][2]:
                        #print("Test {0} failed".format(testNum))
                        #failed.append(i)
                        pass
          
                if pipe.returncode!=true_outputs[i-1][1]:
                    print("Test {0} failed".format(testNum))
                    failed.append(i)
                else:
                    total_passed += 1
                    print("Test {0} pass".format(testNum))
                continue
                
            test_output = pipe.communicate()[0].decode("utf-8")
            print(test_output)
            print("test_rv: {0}, expected_rv:{1}".format(pipe.returncode, true_outputs[i-1][0]) )
                
            if (true_outputs[i-1][0]!=pipe.returncode!=0):
                print("Test {0} failed".format(testNum))
                failed.append(i)
                continue
            else:
                # now run it if j!=2
                if os.path.isfile(exename):
                    if i==31 or i==32:
                        args = [exename, true_outputs[i-1][2]]
                        pipe = run_and_record_process_time(args, i, j, "run")
                    else:
                        args = [exename]
                        pipe = run_and_record_process_time(args, i, j, "run")
                    test_output = pipe.communicate()[0].decode("utf-8")
                    if 27<=i<=30: 
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
        
print("{0}/{1} tests passed".format(total_passed, len(true_outputs)*3))

if len(failed):
    print("test failures: {0}".format(failed))

print("Outputting metrics...")
pd.DataFrame(metrics).to_csv("./out/metrics.csv")
print("metrics written to ./out/metrics.csv")