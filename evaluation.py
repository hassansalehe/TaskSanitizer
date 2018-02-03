#!/usr/bin/env python

import os
import subprocess
import re
import sys
from time import clock

class StrassenOptions(object):
    """
    A class for assembling the compiler options for the Strassen
    benchmark application because:
     (a) It has custom compiler options
     (b) The list of compiler options is long
    """

    def __init__( self ):
        self.cppFiles = [
            "kastors/common/main.c",
            "kastors/strassen/src/strassen-task-dep.c",
            "kastors/strassen/src/strassen.c"
        ]
        self.compilerOptions = [
            "-I./kastors/strassen/include",
            "-I./kastors/common",
            "-DMSIZE", "-DCUTOFF_SIZE", "-DCUTOFF_DEPTH",
            "-lrt", "-lm", "-fpermissive","-D_POSIX_C_SOURCE=199309L"
        ]

    def getFullCommand( self ):
        return self.cppFiles + self.compilerOptions


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
                     "pointer_chasing.cc",
                     "bank_task_racy.cc"];

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
            fpath = self.app_path + app
            print app
            if "strassen" in app:
                options = StrassenOptions()
                fpath = options.getFullCommand()
                commands = ["./flowsan"] + fpath
                print commands
                output, err = self.execute( commands )
            else:
                output, err = self.execute( ["./flowsan", fpath] )

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
        fpath    = self.app_path + appName
        compiler = "/usr/bin/clang++"
        args1    = "-I./bin/include"
        args2    = "-o"
        outName  = appName + "Orig.exe"
        out, err = self.execute( [compiler, fpath, args1, args2, outName] )

        if err:
            sys.exit()

    def compileInstrumentedApp( self, appName ):
        fpath    = self.app_path + appName
        args     = "-o"
        outName  = appName + "Instr.exe"
        out, err = self.execute( ["./flowsan", fpath, args, outName] )

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
