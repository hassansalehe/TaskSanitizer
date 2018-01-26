import os
import subprocess
import re
import sys

class Base(object):
    def initialize( self ):
        self.app_path = "./src/tests/benchmarks/"
        self.apps = os.listdir( self.app_path )
        self.inputSize = "25"
        self.results = []

    def parseArguments( self ):
        if len(sys.argv) > 1:
            self.apps = [sys.argv[1] + ".cc"];
        if(len(sys.argv) > 2):
            self.inputSizes = [sys.argv[2]]

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
            output, err = self.execute( ["./flowsan", fpath] )

            if err is None:
                output, err = self.execute( ["./a.out", self.inputSize] )
                self.formatResult( app, output )

    def formatResult(self, app_name, output):
        num_bugs  = self.findBugs( output )
        num_tasks = self.getNumTasks( output )
        app_name = app_name.replace(".cc", "")
        print app_name,",", self.inputSize,",", num_tasks,",", num_bugs


class Performance( Base ):
    def piece(self):
        pass

if __name__ == "__main__":
    correctness = Correctness()
    correctness.initialize()
    correctness.parseArguments()
    correctness.runExperiments()
