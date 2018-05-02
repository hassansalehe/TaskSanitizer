#!/usr/bin/env python

# Copyright (c) 2017 - 2018, Hassan Salehe Matar
# All rights reserved.
#
# This file is part of TaskSanitizer. For details, see
# https://github.com/hassansalehe/TaskSanitizer. Please also see the LICENSE file
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
        self.benchDir = "./src/tests/benchmarks/"
        self.cppFiles = []
        self.cppArgs  = ["-fopenmp", "-lrt", "-lm", "-O2", "-std=c++11",
                         "-g", "-fpermissive"]

    def getFullCommand( self ):
        return self.cppFiles + self.cppArgs

    def getFormattedInput( self, size ):
        rtSz = str (int( math.sqrt( float(size) ) ) )
        return ["-n", size, "-r", rtSz, "-i", rtSz, "-b", rtSz]
    # end class BenchArgs

class BackgroundExample( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = [self.benchDir + "BackgroundExample.cc"]

class Banking( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = [self.benchDir + "Banking.cc"]

    def getFormattedInput( self, size ):
        return [size]

class Fibonacci( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = [self.benchDir + "Fibonacci.cc"]

    def getFormattedInput( self, size ):
        return [size]

class MapReduce( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = [self.benchDir + "MapReduce.cc"]

    def getFormattedInput( self, size ):
        return self.cppFiles

class PointerChasing( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = [self.benchDir + "PointerChasing.cc"]

    def getFormattedInput( self, size ):
        return [size]

class Sectionslock1OrigNo( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = [self.benchDir + "sectionslock1-orig-no.cc"]

class Taskdep1OrigNo( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = [self.benchDir + "taskdep1-orig-no.cc"]

class Taskdep2OrigNo( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = [self.benchDir + "taskdep2-orig-no.cc"]

class Taskdep3OrigNo( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = [self.benchDir + "taskdep3-orig-no.cc"]

class TaskdependmissingOrigYes( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = [self.benchDir + "taskdependmissing-orig-yes.cc"]

class BenchArgFactory( object ):
    @staticmethod
    def getInstance( benchName ):
        if "BackgroundExample" in benchName:
            return BackgroundExample()
        if "Banking" in benchName:
            return Banking()
        if "Fibonacci" in benchName:
            return Fibonacci()
        if "MapReduce" in benchName:
            return MapReduce()
        if "PointerChasing" in benchName:
           return PointerChasing()
        if "sectionslock1-orig-no" in benchName:
           return Sectionslock1OrigNo()
        if "taskdep1-orig-no" in benchName:
           return Taskdep1OrigNo()
        if "taskdep2-orig-no" in benchName:
           return Taskdep2OrigNo()
        if "taskdep3-orig-no" in benchName:
           return Taskdep3OrigNo()
        if "taskdependmissing-orig-yes" in benchName:
           return TaskdependmissingOrigYes()
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
        self.benchDir = "./src/tests/benchmarks/"
        self.apps = os.listdir( self.benchDir )
        self.results = []
        self.inputSizes = []
        self.apps = ["BackgroundExample",
                     "Banking",
                     "Fibonacci",
                     "MapReduce",
                     "PointerChasing",
                     "sectionslock1-orig-no",
                     "taskdep1-orig-no",
                     "taskdep2-orig-no",
                     "taskdep3-orig-no",
                     "taskdependmissing-orig-yes"];

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
        regex = " memory addresses"
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
            #print app
            name = "./."+ app + "Corr.exe"
            options = BenchArgFactory.getInstance( app )
            commands = ["./tasksan", "-o", name]
            commands.extend( options.getFullCommand() )
            #print commands
            output, err = self.execute( commands )

            if err is None:
                bench    = BenchArgFactory.getInstance( app )
                progArgs = bench.getFormattedInput( str(self.inputSizes[0]) )
                commands = [name] + progArgs
                #print commands
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
        if len(self.apps) > 3:
            self.apps = ["Fibonacci", "MapReduce", "PointerChasing"]

        if len(self.inputSizes) < 1:
            self.inputSizes = [2, 4, 8, 16, 32, 64, 128]

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
        command = ["./tasksan", "-o", outName]
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
        progArgs = bench.getFormattedInput(inputSize)
        command.extend( progArgs )
        print command
        start = clock()
        out, err = self.execute( command )
        return (clock() - start)

    def runInstrumentedApp( self, appName, inputSize ):
        name     = "./" + appName + "Instr.exe"
        command  = [name]
        bench    = BenchArgFactory.getInstance(appName)
        progArgs = bench.getFormattedInput(inputSize)
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
        print "         BackgroundExample"
        print "         Banking"
        print "         Fibonacci"
        print "         MapReduce"
        print "         PointerChasing"
        print "         sectionslock1-orig-no"
        print "         taskdep1-orig-no"
        print "         taskdep2-orig-no"
        print "         taskdep3-orig-no"
        print "         taskdependmissing-orig-yes"
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
