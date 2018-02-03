#!/usr/bin/env python

# Copyright (c) 2017 - 2018, Hassan Salehe Matar
# All rights reserved.
#
# This file is part of FlowSanitizer. For details, see
# https://github.com/hassansalehe/FlowSanitizer. Please also see the LICENSE file
# for additional BSD notice
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import subprocess
import re
import sys
from time import clock

###############################################################
### Classes below manage benchmark specific arguments
###############################################################
class BenchArgs(object):
    """
    Base class for holding command arguments for specific
    benchmark applications.
        A class for assembling the compiler options for the
        benchmark applications because:
         (a) Each may have custom compiler options
         (b) The list of compiler options may be long
    """
    def __init__(self):
        self.cppFiles = []
        self.cppArgs  = ["-fopenmp", "-lrt", "-lm", "-O2", "-g",
                         "-fpermissive", "-DMSIZE", "-DCUTOFF_SIZE",
                         "-DCUTOFF_DEPTH", "-D_POSIX_C_SOURCE=199309L",
                         "-DBSIZE", "-DTITER", "-I./kastors/common"]

    def getFullCommand( self ):
        return self.cppFiles + self.cppArgs

class Fibonacci( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = ["./src/tests/benchmarks/fibonacci.cc"]

class PointerChasing( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = ["./src/tests/benchmarks/pointer_chasing.cc"]

class BankTaskRacy( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = ["./src/tests/benchmarks/bank_task_racy.cc"]

class Strassen(BenchArgs):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles.extend( ["kastors/common/main.c",
            "kastors/strassen/src/strassen-task-dep.c",
            "kastors/strassen/src/strassen.c"] )
        self.cppArgs.extend( ["-I./kastors/strassen/include"] )

class Jacobi(BenchArgs):
    def __init__( self ):
        BenchArgs.__init__(self)
        dir = "kastors/jacobi/src/"
        self.cppFiles.extend( ["kastors/common/main.c",
            dir + "jacobi-task-dep.c",
            dir + "poisson.c",
            dir + "jacobi-seq.c"] )
        self.cppArgs.extend( ["-I./kastors/jacobi/include"] )

class SparseLU(BenchArgs):
    def __init__( self ):
        BenchArgs.__init__(self)
        dir = "kastors/sparselu/src/"
        self.cppFiles.extend( ["kastors/common/main.c",
            dir + "sparselu-task-dep.c",
            dir + "sparselu.c",
            dir + "sparselu-seq.c"] )
        self.cppArgs.extend( ["-I./kastors/sparselu/include", "-DSMSIZE"] )

class Plasma(BenchArgs):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles.extend( [
            "./kastors/common/main.c",
            "./kastors/plasma/src/auxiliary.c",
            "./kastors/plasma/src/core_dgeqrt.c",
            "./kastors/plasma/src/core_dgetrf_rectil.c",
            "./kastors/plasma/src/pdgetrf_rectil.c",
            "./kastors/plasma/src/core_dlaswp.c",
            "./kastors/plasma/src/core_dormqr.c",
            "./kastors/plasma/src/core_dparfb.c",
            "./kastors/plasma/src/core_dpamm.c",
            "./kastors/plasma/src/core_dplgsy.c",
            "./kastors/plasma/src/core_dplrnt.c",
            "./kastors/plasma/src/core_dtsmqr.c",
            "./kastors/plasma/src/core_dtsqrt.c",
            "./kastors/plasma/src/dauxiliary.c",
            "./kastors/plasma/src/descriptor.c",
            "./kastors/plasma/src/dgeqrs.c",
            "./kastors/plasma/src/dgetrs.c",
            "./kastors/plasma/src/dpotrs.c",
            "./kastors/plasma/src/global.c",
            "./kastors/plasma/src/pdgeqrf.c",
            "./kastors/plasma/src/pdlaswp.c",
            "./kastors/plasma/src/pdormqr.c",
            "./kastors/plasma/src/pdplgsy.c",
            "./kastors/plasma/src/pdpltmg.c",
            "./kastors/plasma/src/pdpotrf.c",
            "./kastors/plasma/src/pdtile.c",
            "./kastors/plasma/src/pdtrsm.c",
            "./kastors/plasma/src/workspace.c"
            ] )

        self.cppArgs.extend( ["-I./kastors/plasma/include", "-DADD_",
            "-llapacke", "-lblas", "-llapack", "-I./kastors/plasma",
            "-DMSIZE", "-DBSIZE", "-DGFLOPS"] )

class Dgeqrf(Plasma):
    def __init__(self):
        Plasma.__init__(self)
        self.cppArgs.extend(["./kastors/plasma/src/time_dgeqrf-task.c"])

class Dgetrf(Plasma):
    def __init__(self):
        Plasma.__init__(self)
        self.cppFiles.extend(["./kastors/plasma/src/time_dgetrf-task.c"])

class Dpotrf(Plasma):
    def __init__(self):
        Plasma.__init__(self)
        self.cppFiles.extend(["./kastors/plasma/src/time_dpotrf-task.c"])

class BenchArgFactory(object):
    @staticmethod
    def getArgInstance( benchName):
        if "strassen" in benchName:
            return Strassen()
        if "fibonacci" in benchName:
            return Fibonacci()
        if "pointer_chasing" in benchName:
           return PointerChasing()
        if "bank_task_racy" in benchName:
            return BankTaskRacy()
        if "jacobi" in benchName:
            return Jacobi()
        if "sparselu" in benchName:
            return SparseLU()
        if "dgeqrf" in benchName:
            return Dgeqrf()
        if "dgetrf" in benchName:
            return Dgetrf()
        if "dpotrf" in benchName:
            return Dpotrf()
        # else: no such penchmark
        print "No such a benchmark", benchName
        print "Exiting ..."
        sys.exit()

###############################################################
### Classes below are responsible for running experiments
###############################################################
class Base(object):
    """
    This base class implements the base functionalities used by both
    correctness and performance experiments.
    """
    def __init__( self ):
        self.app_path = "./src/tests/benchmarks/"
        self.apps = os.listdir( self.app_path )
        self.results = []
        self.inputSizes = []
        self.apps = ["strassen",
                     "fibonacci.cc",
                     "pointer_chasing",
                     "bank_task_racy",
                     "jacobi",
                     "sparselu",
                     "dgeqrf",
                     "dgetrf",
                     "dpotrf"];

        if len(sys.argv) > 1:
            self.experiment = sys.argv[1]
        if len(sys.argv) > 2:
            self.apps = [sys.argv[2] + ".cc"];
        if(len(sys.argv) > 3):
            self.inputSizes = [sys.argv[3]]

    def execute( self, commands):
        p = subprocess.Popen(commands, stdout=subprocess.PIPE)
        return p.communicate()


class Correctness( Base ):
    """
    This class implements the functions for running instrumented version
    of the benchmark applications for reporting of bugs from them.

    It is launced if you provide "correctness" as the first argument
    if the evaluation Python script.

    It works in three modes:
       (1) It runs all the benchmark programs if no specific benchmark
           is specified. Example command: ./evaluation.py correctness
       (2) It runs specified benchmark application.
           Example: ./evaluation.py correctness fibonacci
       (3) It runs a specified benchmark application with a speficied
           input size. E.g; ./evaluation.py correctness fibonacci 400
    """

    def __init__( self ):
        super(Correctness, self).__init__()
        if len (self.inputSizes) < 1:
            self.inputSizes.append(16)

    def findBugs( self, report):
        regex = "lines:"
        bugs = 0
        for line in report.splitlines():
            if regex in line:
                bugs += 1
        return bugs

    def getNumTasks( self, report):
        regex = "Total number of tasks:"
        bugs = 0
        for line in report.splitlines():
            if regex in line:
                return re.findall('\d+', line)[0]

    def runExperiments( self ):
        head = "Application, Input size, # tasks, # bugs"
        print head
        for app in self.apps:
            print app
            options = BenchArgFactory.getArgInstance( app )
            commands = ["./flowsan"]
            commands.extend( options.getFullCommand() )
            print commands
            output, err = self.execute( commands )

            if err is None:
                output, err = self.execute( ["./a.out", str(self.inputSizes[0])] )
                self.formatResult( app, self.inputSizes[0], output )

    def formatResult(self, appName, inputSize, output):
        num_bugs  = self.findBugs( output )
        num_tasks = self.getNumTasks( output )
        appName  = appName.replace(".cc", "")
        print appName,",", inputSize,",", num_tasks,",", num_bugs


class Performance( Base ):
    """
    The class for running experiments for
    """
    def __init__(self):
        super(Performance, self).__init__()
        self.repetitions = 10
        if len(self.inputSizes) < 1:
            self.inputSizes = [2, 4, 8, 16]

    def compileOriginalApp( self, appName ):
        outName  = appName + "Orig.exe"
        command = ["/usr/bin/clang++", "-I./bin/include", "-o", outName]
        args = BenchArgFactory.getArgInstance( appName ).getFullCommand()
        command.extend( args )
        print command
        out, err = self.execute( command )

        if err:
            sys.exit()

    def compileInstrumentedApp( self, appName ):
        outName  = appName + "Instr.exe"
        command = ["./flowsan", "-o", outName]
        args = BenchArgFactory.getArgInstance( appName ).getFullCommand()
        command.extend( args )
        print command
        out, err = self.execute( command )

        if err:
            sys.exit()

    def runOriginalApp( self, appName, inputSize ):
        name = "./" + appName + "Orig.exe"
        start = clock()
        out, err = self.execute( [name, inputSize] )
        return (clock() - start)

    def runInstrumentedApp( self, appName, inputSize ):
        name = "./" + appName + "Instr.exe"
        start = clock()
        out, err = self.execute( [name, inputSize] )
        return (clock() - start)

    def runExperiments( self ):
        self.finalResults = {}
        for app in self.apps:
            self.compileOriginalApp( app )
            self.compileInstrumentedApp( app )

            result = []
            for inputSz in self.inputSizes:
                origExecTime  = 0;
                instrExecTime = 0;

                # run the original application
                for iter in range( self.repetitions ):
                    execTime     = self.runOriginalApp( app, str(inputSz) )
                    origExecTime = origExecTime + execTime
                if origExecTime > 0 :
                    origExecTime = origExecTime / self.repetitions

                # run the instrumented application
                for iter in range( self.repetitions ):
                    execTime      = self.runInstrumentedApp( app, str(inputSz) )
                    instrExecTime = instrExecTime + execTime
                if origExecTime > 0:
                    instrExecTime = instrExecTime / self.repetitions

                # calculate slowdown
                if origExecTime > 0:
                    slowdown = instrExecTime / origExecTime
                    result.append( (inputSz, slowdown) )
                else:
                    print "Execution time zero"
            self.finalResults[app] = result

        self.formatResult()


    def formatResult(self):
        for appName in self.finalResults:
            print appName
            for iSize, sDown in self.finalResults[appName]:
                print "  ("+str(iSize)+", "+ str(sDown)+")",
            print


class Help( object ):
    def __init__(self):
        print "./evaluation.py <experiment> <application> <input size>"
        print ""
        print "    <experiment> is \"correctness\" or \"performance\" "
        print "    <application> can be one of:",
        print "  bank_task_racy, fibonacci, pointer_chasing"
        print ""
        print " NOTE:"
        print "   1. If you do not specify input size, your application"
        print "      will run with default input."
        print "   2. If you do not specify application, all benchmark"
        print "      applications will be executed."

if __name__ == "__main__":
    # parse arguments
    if len(sys.argv) > 1:
        option = sys.argv[1]
        if option == "correctness":
            correctness = Correctness()
            correctness.runExperiments()
        elif option == "performance":
            performance = Performance()
            performance.runExperiments()
            print "Performance"
        else:
            print "Wrong commands:", sys.argv
            Help()
    else: # no arguments means run both correctness and peformance
        print "Wrong commands:", sys.argv
        correctness = Correctness()
        correctness.runExperiments()
        performance = Performance()
        performance.runExperiments()
