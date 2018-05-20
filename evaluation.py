#!/usr/bin/env python

# Copyright (c) 2015 - 2018 Hassan Salehe Matar
# All rights reserved.
#
# This file is part of TaskSanitizer. For details, see
# https://github.com/hassansalehe/TaskSanitizer. Please also see the
# LICENSE file for additional BSD notice
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
import re
import sys
import math
import subprocess
import numpy as np
from time import clock
import matplotlib.pyplot as plt

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
        self.benchDir = "./src/benchmarks/"
        self.cppFiles = []
        self.cppArgs  = ["-fopenmp", "-lrt", "-lm", "-O2", "-std=c++11",
                         "-g", "-fpermissive"]

    def getFullCommand( self ):
        return self.cppFiles + self.cppArgs

    def getFormattedInput( self, size ):
        rtSz = str (int( math.sqrt( float(size) ) ) )
        return ["-n", size, "-r", rtSz, "-i", rtSz, "-b", rtSz]
    # end class BenchArgs

class RacyBackgroundExample( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = [self.benchDir + "RacyBackgroundExample.cc"]

class RacyBanking( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = [self.benchDir + "RacyBanking.cc"]

    def getFormattedInput( self, size ):
        return [size]

class RacyFibonacci( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = [self.benchDir + "RacyFibonacci.cc"]

    def getFormattedInput( self, size ):
        return [size]

class RacyMapReduce( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = [self.benchDir + "RacyMapReduce.cc"]

    def getFormattedInput( self, size ):
        return self.cppFiles

class RacyPointerChasing( BenchArgs ):
    def __init__( self ):
        BenchArgs.__init__(self)
        self.cppFiles = [self.benchDir + "RacyPointerChasing.cc"]

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
        if "RacyBackgroundExample" in benchName:
            return RacyBackgroundExample()
        if "RacyBanking" in benchName:
            return RacyBanking()
        if "RacyFibonacci" in benchName:
            return RacyFibonacci()
        if "RacyMapReduce" in benchName:
            return RacyMapReduce()
        if "RacyPointerChasing" in benchName:
           return RacyPointerChasing()
        if "sectionslock1-orig-no" in benchName:
           return Sectionslock1OrigNo()
        if "taskdep1-orig-no" in benchName:
           return Taskdep1OrigNo()
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
        self.benchDir = "./src/benchmarks/"
        self.apps = os.listdir( self.benchDir )
        self.results = []
        self.inputSizes = []
        self.apps = ["RacyBackgroundExample",
                     "RacyBanking",
                     "RacyFibonacci",
                     "RacyMapReduce",
                     "RacyPointerChasing",
                     "sectionslock1-orig-no",
                     "taskdep1-orig-no",
                     "taskdep3-orig-no",
                     "taskdependmissing-orig-yes"];

        if len(sys.argv) > 1:
            self.experiment = sys.argv[1]
        if len(sys.argv) > 2:
            self.apps = [sys.argv[2] + ".cc"];
        if(len(sys.argv) > 3):
            self.inputSizes = [sys.argv[3]]

    def getLibraryPath( self ):
        taskSanHomeDir = os.path.dirname(os.path.abspath(__file__))
        return taskSanHomeDir + "/bin/lib"

    def getIncludePath( self ):
        taskSanHomeDir = os.path.dirname(os.path.abspath(__file__))
        return taskSanHomeDir + "/bin/include"

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
        regex = " task pairs have conflicts:"
        bugs = 0
        for line in report.splitlines():
            if regex in line:
                for word in line.split():
                    if word.isdigit():
                        bugs = int(word)
                        break
                break
        return bugs

    def getNumTasks( self, report ):
        regex = "Total number of tasks:"
        bugs = 0
        for line in report.splitlines():
            if regex in line:
                return re.findall('\d+', line)[0]

    def runExperiments( self ):
        print ""
        print "Showing number of bugs detected from a set of micro-benchmark applications"
        print ""
        head = ["Application", "Input size", "# tasks", "# bugs found"]
        row_format ="| {:<27}| {:<11}| {:<8}| {:<13}|"
        print row_format.format(*head)
        for app in self.apps:
            #print app
            name = "./."+ app + "Corr.exe"
            if os.path.isfile(name):
                os.remove(name)
            options = BenchArgFactory.getInstance( app )
            commands = ["./tasksan", "-o", name]
            commands.extend( options.getFullCommand() )
            #print commands
            output, err = self.execute( commands )

            if err is None and os.path.isfile(name):
                bench    = BenchArgFactory.getInstance( app )
                progArgs = bench.getFormattedInput( str(self.inputSizes[0]) )
                commands = [name] + progArgs
                #print commands
                output, err = self.execute( commands )
                if err is None:
                    self.formatResult( app, self.inputSizes[0], output )

    def formatResult( self, appName, inputSize, output ):
        num_bugs  = self.findBugs( output )
        num_tasks = self.getNumTasks( output )
        appName  = appName.replace(".cc", "")
        row = [appName, inputSize, num_tasks, num_bugs]
        row_format ="| {:<27}| {:<11}| {:<8}| {:<13}|"
        print row_format.format(*row)

class ArcherCorrectness( Correctness ):
    """
    This class implements the functions for running instrumented version
    of the benchmark applications for reporting of bugs from them.

    It is launced if you provide "correctness" as the first argument
    if the evaluation Python script.

    It works in three modes:
       (1) It runs all the benchmark programs if no specific benchmark
           is specified. Example command: ./evaluation.py archer
       (2) It runs specified benchmark application.
           Example: ./evaluation.py archer fibonacci
       (3) It runs a specified benchmark application with a speficied
           input size. E.g; ./evaluation.py archer fibonacci 400
    """

    def __init__( self ):
        Correctness.__init__(self)

    def archerExecute( self, commands ):
        p = subprocess.Popen(commands, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return p.communicate()

    def archerFindBugs( self, report ):
        regex = "ThreadSanitizer: reported "
        bugs = 0
        for line in report.splitlines():
            if regex in line and "warnings" in line:
                for word in line.split():
                    if word.isdigit():
                        bugs = int(word)
                        break
                break
        return bugs

    def archerCompile( self, app ):
        #print app
        name = "./."+ app + "archerCorr.exe"
        if os.path.isfile(name):
            os.remove(name)
        options = BenchArgFactory.getInstance( app )
        commands = ["./bin/bin/clang-archer++", "-o", name]
        commands.extend( options.getFullCommand() )
        commands.extend(["-L./bin/lib", "-larcher"])
        #print commands
        return self.execute( commands )

    def archerFormatResult( self, appName, inputSize, output ):
        num_bugs  = self.archerFindBugs( output )
        appName  = appName.replace(".cc", "")
        row = [num_bugs]
        row_format ="{:<13}|" * (len(row))
        return row_format.format(*row)

    def runArcher(self, app):
        name = "./."+ app + "archerCorr.exe"
        output, err = self.archerCompile( app )

        if err is None and os.path.isfile(name):
            bench    = BenchArgFactory.getInstance( app )
            progArgs = bench.getFormattedInput( str(self.inputSizes[0]) )
            commands = [name] + progArgs
            #print commands
            output, err = self.archerExecute( commands )
            if err is not None:
                return self.archerFormatResult( app, self.inputSizes[0], err )
        return []

    def runTaskSanitizer( self, app ):
        #print app
        name = "./."+ app + "Corr.exe"
        if os.path.isfile(name):
            os.remove(name)
        options = BenchArgFactory.getInstance( app )
        commands = ["./tasksan", "-o", name]
        commands.extend( options.getFullCommand() )
        #print commands
        output, err = self.execute( commands )

        if err is None and os.path.isfile(name):
            bench    = BenchArgFactory.getInstance( app )
            progArgs = bench.getFormattedInput( str(self.inputSizes[0]) )
            commands = [name] + progArgs
            #print commands
            output, err = self.execute( commands )
            if err is None:
                return self.formatTasanResult( app, self.inputSizes[0], output )
        return []

    def formatTasanResult( self, appName, inputSize, output ):
        num_bugs  = self.findBugs( output )
        num_tasks = self.getNumTasks( output )
        appName  = appName.replace(".cc", "")
        row = [appName, inputSize, num_tasks, num_bugs]
        row_format ="| {:<27}| {:<11}| {:<8}| {:<21}|"
        return row_format.format(*row)

    def runExperiments( self ):
        print ""
        print "Comparing detection results with that of Archer on same micro-benchmarks"
        print ""
        head = ["Application", "Input size", "# tasks", "# bugs TaskSanitizer", "# bugs Archer"]
        row_format ="| {:<27}| {:<11}| {:<8}| {:<21}| {:<13}|" #* (len(head))
        print row_format.format(*head)
        for app in self.apps:
            #print app
            tasanResult = self.runTaskSanitizer(app)
            archerResult = self.runArcher(app)
            print tasanResult, archerResult


class Performance( Experiment ):
    """
    The class for running experiments for calculating
    the slowdown of runtime nondeterminism detection.

    It uses a set of different input sizes to access the
    slowdown of a benchmark program.
    """
    def __init__( self ):
        Experiment.__init__(self)
        self.repetitions = 20
        if len(self.apps) > 3:
            self.apps = ["RacyFibonacci", "RacyMapReduce", "RacyPointerChasing"]

        if len(self.inputSizes) < 1:
            self.inputSizes = [2, 4, 8, 16, 32, 64, 128]
        print ""
        print "Running performance evaluation. Be patient as it takes time!"
        print ""

    def compileOriginalApp( self, appName ):
        outName  = "./." + appName + "Orig.exe"
        command = ["/usr/bin/clang++"]
        command.append( "-L" + self.getLibraryPath() )
        command.append( "-Wl,-rpath=" + self.getLibraryPath() )
        command.append( "-I" + self.getIncludePath() )
        command.append( "-o" )
        command.append( outName )
        args = BenchArgFactory.getInstance( appName ).getFullCommand()
        command.extend( args )
        #print command
        out, err = self.execute( command )

        if err:
            sys.exit()

    def compileInstrumentedApp( self, appName ):
        outName  = "./." + appName + "Instr.exe"
        command = ["./tasksan", "-o", outName]
        args = BenchArgFactory.getInstance( appName ).getFullCommand()
        command.extend( args )
        print command
        out, err = self.execute( command )

        if err:
            sys.exit()

    def runOriginalApp( self, appName, inputSize ):
        name     = "./." + appName + "Orig.exe"
        command  = [name]
        bench    = BenchArgFactory.getInstance(appName)
        progArgs = bench.getFormattedInput(inputSize)
        command.extend( progArgs )
        #print command
        start = clock()
        out, err = self.execute( command )
        return (clock() - start)

    def runInstrumentedApp( self, appName, inputSize ):
        name     = "./." + appName + "Instr.exe"
        command  = [name]
        bench    = BenchArgFactory.getInstance(appName)
        progArgs = bench.getFormattedInput(inputSize)
        command.extend( progArgs )
        #print command
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
        plt.figure("Slowdown of determinacy race detection in programs as input size increases", figsize=(12, 10))
        number = len(self.finalResults) * 10 + 101
        for appName in self.finalResults:
            inputx    = []
            slowdowny = []
            figure    = plt.subplot(number)
            number    = number + 1
            figure.set_title(appName)
            figure.set_xlabel("Input size (n)")
            figure.set_ylabel("Slowdown")
            print appName
            for iSize, sDown in self.finalResults[appName]:
                inputx.append(iSize)
                slowdowny.append(sDown)
                print "("+str(iSize)+", "+ str(sDown)+")",
            print
            figure.plot(inputx, slowdowny, 'bo', inputx, slowdowny, 'k')
        plt.show()

    # end class performance

class Help( object ):
    """
    Help is invoked when user supplies wrong command
    arguments to the script
    """
    def __init__( self ):
        print "./evaluation.py <experiment> <application> <input size>"
        print ""
        print "    <experiment> is \"correctness\" or \"performance\" or \"archer\""
        print "    <application> can be one of:"
        print "         RacyBackgroundExample"
        print "         RacyBanking"
        print "         RacyFibonacci"
        print "         RacyMapReduce"
        print "         RacyPointerChasing"
        print "         sectionslock1-orig-no"
        print "         taskdep1-orig-no"
        print "         taskdep3-orig-no"
        print "         taskdependmissing-orig-yes"
        print ""
        print " NOTE:"
        print "   1. If you do not specify input size, your application"
        print "      will run with default input."
        print "   2. If you do not specify application, all benchmark"
        print "      applications will be executed."
        print "   3. If you do not specify <experiment>, all evaluation"
        print "      experiments will be executed."
    # end class Help

###############################################################
### Below is the main code of the program
###############################################################
if __name__ == "__main__":
    # parse arguments
    print "TaskSanitizer Evaluation Script"
    if len(sys.argv) > 1:
        option = sys.argv[1]
        if option == "correctness":
            correctness = Correctness()
            correctness.runExperiments()
        elif option == "performance":
            performance = Performance()
            performance.runExperiments()
            print "Performance"
        elif option == "archer":
            correctness = ArcherCorrectness()
            correctness.runExperiments()
        elif option == "help":
            Help()
        else:
            print "Wrong commands:", sys.argv
            Help()
    else: # no arguments means run both correctness and peformance
        print "Running all experiments:", sys.argv
        correctness = Correctness()
        correctness.runExperiments()
        archerCorrect = ArcherCorrectness()
        archerCorrect.runExperiments()
        performance = Performance()
        performance.runExperiments()
