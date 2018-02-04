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
import sys
import subprocess
import re
import math
from time import clock

###############################################################
### Classes below manage benchmark specific arguments
###############################################################
class BenchArgs( object ):
    """
    Base class for holding command arguments for specific
    benchmark applications.
        A class for assembling the compiler options for the
        benchmark applications because:
         (a) Each may have custom compiler options
         (b) The list of compiler options may be long
    """
    def __init__( self ):
        self.cppFiles = []
        self.cppArgs  = ["-fopenmp", "-lrt", "-lm", "-O2", "-g",
                         "-fpermissive", "-DMSIZE", "-DCUTOFF_SIZE",
                         "-DCUTOFF_DEPTH", "-D_POSIX_C_SOURCE=199309L",
                         "-DBSIZE", "-DTITER", "-I./kastors/common"]

    def getFullCommand( self ):
        return self.cppFiles + self.cppArgs

    def getFromattedInput( self, size ):
        pass
    # end class BenchArgs

class Fibonacci( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = ["./src/tests/benchmarks/fibonacci.cc"]

    def getFromattedInput( self, size ):
        return [size]

class PointerChasing( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = ["./src/tests/benchmarks/pointer_chasing.cc"]

    def getFromattedInput( self, size ):
        return [size]

class BankTaskRacy( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = ["./src/tests/benchmarks/bank_task_racy.cc"]

    def getFromattedInput( self, size ):
        return [size]

class Strassen( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles.extend( ["kastors/common/main.c",
            "kastors/strassen/src/strassen-task-dep.c",
            "kastors/strassen/src/strassen.c"] )
        self.cppArgs.extend( ["-I./kastors/strassen/include"] )

    def getFromattedInput( self, size ):
        blockSz = str (int( math.sqrt( float(size) ) ) )
        return ["-n", size, "-r", size, "-i", size, "-b", blockSz]

    # end class Strassen

class Jacobi( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        dir = "kastors/jacobi/src/"
        self.cppFiles.extend( ["kastors/common/main.c",
            dir + "jacobi-task-dep.c",
            dir + "poisson.c",
            dir + "jacobi-seq.c"] )
        self.cppArgs.extend( ["-I./kastors/jacobi/include"] )

    def getFromattedInput( self, size ):
        blockSz = str (int( math.sqrt( float(size) ) ) )
        return ["-n", size, "-r", size, "-i", size, "-b", blockSz]
    # end class Jacobi

class SparseLU( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        dir = "kastors/sparselu/src/"
        self.cppFiles.extend( ["kastors/common/main.c",
            dir + "sparselu-task-dep.c",
            dir + "sparselu.c",
            dir + "sparselu-seq.c"] )
        self.cppArgs.extend( ["-I./kastors/sparselu/include", "-DSMSIZE"] )

    def getFromattedInput( self, size ):
        blockSz = str (int( math.sqrt( float(size) ) ) )
        return ["-n", size, "-r", size, "-i", size, "-b", blockSz, "-m", blockSz]
    # end class SparseLU

class Plasma( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.dir = "./kastors/plasma/src/"
        self.cppFiles.extend( [
            "./kastors/common/main.c",
            self.dir + "auxiliary.c",
            self.dir + "core_dgeqrt.c",
            self.dir + "core_dgetrf_rectil.c",
            self.dir + "pdgetrf_rectil.c",
            self.dir + "core_dlaswp.c",
            self.dir + "core_dormqr.c",
            self.dir + "core_dparfb.c",
            self.dir + "core_dpamm.c",
            self.dir + "core_dplgsy.c",
            self.dir + "core_dplrnt.c",
            self.dir + "core_dtsmqr.c",
            self.dir + "core_dtsqrt.c",
            self.dir + "dauxiliary.c",
            self.dir + "descriptor.c",
            self.dir + "dgeqrs.c",
            self.dir + "dgetrs.c",
            self.dir + "dpotrs.c",
            self.dir + "global.c",
            self.dir + "pdgeqrf.c",
            self.dir + "pdlaswp.c",
            self.dir + "pdormqr.c",
            self.dir + "pdplgsy.c",
            self.dir + "pdpltmg.c",
            self.dir + "pdpotrf.c",
            self.dir + "pdtile.c",
            self.dir + "pdtrsm.c",
            self.dir + "workspace.c"
            ] )

        self.cppArgs.extend( ["-I./kastors/plasma/include", "-DADD_",
            "-llapacke", "-lblas", "-llapack", "-I./kastors/plasma",
            "-DMSIZE", "-DBSIZE", "-DGFLOPS"] )

class Dgeqrf( Plasma ):
    def __init__( self ):
        Plasma.__init__(self)
        self.cppArgs.extend(["-DIBSIZE"]);
        self.cppFiles.extend([self.dir + "time_dgeqrf-task.c"])

    def getFromattedInput( self, size ):
        blockSz = str (int( math.sqrt( float(size) ) ) )
        return ["-n", size, "-r", size, "-i", size, "-b", blockSz, "-ib", blockSz]

class Dgetrf( Plasma ):
    def __init__( self ):
        Plasma.__init__(self)
        self.cppFiles.extend([self.dir + "time_dgetrf-task.c"])

    def getFromattedInput( self, size ):
        blockSz = str (int( math.sqrt( float(size) ) ) )
        return ["-n", size, "-r", size, "-i", size, "-b", blockSz]

class Dpotrf( Plasma ):
    def __init__( self ):
        Plasma.__init__(self)
        self.cppFiles.extend([self.dir + "time_dpotrf-task.c"])

    def getFromattedInput( self, size ):
        blockSz = str (int( math.sqrt( float(size) ) ) )
        return ["-n", size, "-r", size, "-i", size, "-b", blockSz]

class BenchArgFactory( object ):
    @staticmethod
    def getInstance( benchName ):
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
        # else:
        # no such penchmark
        print "No such a benchmark", benchName
        Help()
        print "Exiting ..."
        sys.exit()
    # end class BenchArgFactory

###############################################################
### Classes below are responsible for running experiments
###############################################################
class Experiment( object ):
    """
    This base class implements the base functionalities used by both
    correctness and performance experiments.
    """
    def __init__( self ):
        self.app_path = "./src/tests/benchmarks/"
        self.apps = os.listdir( self.app_path )
        self.results = []
        self.inputSizes = []
        self.apps = ["fibonacci",
                     "pointer_chasing",
                     "bank_task_racy",
                     "jacobi",
                     "sparselu",
                     "strassen",
                     "dgeqrf",
                     "dgetrf",
                     "dpotrf"];

        if len(sys.argv) > 1:
            self.experiment = sys.argv[1]
        if len(sys.argv) > 2:
            self.apps = [sys.argv[2] + ".cc"];
        if(len(sys.argv) > 3):
            self.inputSizes = [sys.argv[3]]

    def execute( self, commands ):
        p = subprocess.Popen(commands, stdout=subprocess.PIPE)
        return p.communicate()
    # end class Experiment

class Correctness( Experiment ):
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
        Experiment.__init__(self)
        if len (self.inputSizes) < 1:
            self.inputSizes = [16]

    def findBugs( self, report ):
        regex = "lines:"
        bugs = 0
        for line in report.splitlines():
            if regex in line:
                bugs += 1
        return bugs

    def getNumTasks( self, report ):
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
            name = app + "Perf.exe"
            options = BenchArgFactory.getInstance( app )
            commands = ["./flowsan", "-o", name]
            commands.extend( options.getFullCommand() )
            print commands
            output, err = self.execute( commands )

            if err is None:
                bench    = BenchArgFactory.getInstance( app )
                progArgs = bench.getFromattedInput( str(self.inputSizes[0]) )
                commands = [name] + progArgs
                print commands
                output, err = self.execute( commands )
                self.formatResult( app, self.inputSizes[0], output )

    def formatResult( self, appName, inputSize, output ):
        num_bugs  = self.findBugs( output )
        num_tasks = self.getNumTasks( output )
        appName  = appName.replace(".cc", "")
        print appName,",", inputSize,",", num_tasks,",", num_bugs


class Performance( Experiment ):
    """
    The class for running experiments for calculating
    the slowdown of runtime nondeterminism detection.

    It uses a set of different input sizes to access the
    slowdown of a benchmark program.
    """
    def __init__( self ):
        Experiment.__init__(self)
        self.repetitions = 10
        if len(self.inputSizes) < 1:
            self.inputSizes = [2, 4, 8, 16]

    def compileOriginalApp( self, appName ):
        outName  = appName + "Orig.exe"
        command = ["/usr/bin/clang++", "-I./bin/include", "-o", outName]
        args = BenchArgFactory.getInstance( appName ).getFullCommand()
        command.extend( args )
        print command
        out, err = self.execute( command )

        if err:
            sys.exit()

    def compileInstrumentedApp( self, appName ):
        outName  = appName + "Instr.exe"
        command = ["./flowsan", "-o", outName]
        args = BenchArgFactory.getInstance( appName ).getFullCommand()
        command.extend( args )
        print command
        out, err = self.execute( command )

        if err:
            sys.exit()

    def runOriginalApp( self, appName, inputSize ):
        name     = "./" + appName + "Orig.exe"
        command  = [name]
        bench    = BenchArgFactory.getInstance(appName)
        progArgs = bench.getFromattedInput(inputSize)
        command.extend( progArgs )
        print command
        start = clock()
        out, err = self.execute( command )
        return (clock() - start)

    def runInstrumentedApp( self, appName, inputSize ):
        name     = "./" + appName + "Instr.exe"
        command  = [name]
        bench    = BenchArgFactory.getInstance(appName)
        progArgs = bench.getFromattedInput(inputSize)
        command.extend( progArgs )
        print command
        start = clock()
        out, err = self.execute( command )
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
                    slowdown = round( instrExecTime / origExecTime, 2)
                    result.append( (inputSz, slowdown) )
                else:
                    print "Execution time zero"
            self.finalResults[app] = result

        self.formatResult()


    def formatResult( self ):
        for appName in self.finalResults:
            print appName
            for iSize, sDown in self.finalResults[appName]:
                print "("+str(iSize)+", "+ str(sDown)+")",
            print
    # end class performance

class Help( object ):
    """
    Help is invoked when user supplies wrong command
    arguments to the script
    """
    def __init__( self ):
        print "./evaluation.py <experiment> <application> <input size>"
        print ""
        print "    <experiment> is \"correctness\" or \"performance\" "
        print "    <application> can be one of:"
        print "         fibonacci"
        print "         pointer_chasing"
        print "         bank_task_racy"
        print "         jacobi"
        print "         sparselu"
        print "         strassen"
        print "         dgeqrf"
        print "         dgetrf"
        print "         dpotrf"
        print ""
        print " NOTE:"
        print "   1. If you do not specify input size, your application"
        print "      will run with default input."
        print "   2. If you do not specify application, all benchmark"
        print "      applications will be executed."
    # end class Help

###############################################################
### Below is the main code of the program
###############################################################
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
