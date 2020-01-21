#!/usr/bin/env python
# encoding: utf-8

'''
SiQAD plugin which takes DB locations and return SiQAD commands instructing 
SiQAD to form an aggregate for each DB-pair. DBs which are already in 
aggregates are ignored.
'''

__author__      = 'Samuel Ng'
__copyright__   = 'Apache License 2.0'
__version__     = '0.1'
__date__        = '2019-03-13'  # last update

from argparse import ArgumentParser
import os.path
import numpy as np
from scipy.spatial import distance

import siqadconn

class SiQADInterface:
    '''Interface with SiQAD using SiQADConnector and command line arguments.'''

    dbs = []        # list of tuples of floats containing DB locations (x, y)

    def __init__(self):
        '''Initialize the interface.'''
        self.parse_cml_args()
        self.sqconn = siqadconn.SiQADConnector("DBPairRecognizer", 
                self.args.in_file, self.args.out_file)

    def exec(self):
        '''Execute the recognition process.
        
        Returns:
            An integer indicating the exit state (0 for clean, 1 for error).
        '''
        # TODO exception checks 
        self.init_problem()
        self.perform_recognition()
        self.export_results()
        return 0

    def parse_cml_args(self):
        '''Parse command line arguments.'''

        def file_must_exist(fpath):
            '''Check if input file exists for argument parser'''
            if not os.path.exists(fpath):
                raise argparse.ArgumentTypeError("{0} does not exist".format(fpath))
            return fpath

        parser = ArgumentParser(description="SiQAD plugin which takes DB "
                "locations and return SiQAD commands instructing SiQAD to form "
                "an aggregate for each DB-pair. DBs which are already in "
                "aggregates are ignored.")
        parser.add_argument(dest="in_file", type=file_must_exist,
                help="Path to the problem file.", metavar="IN_FILE")
        parser.add_argument(dest="out_file", help="Path to the output file.",
                metavar="OUT_FILE")
        self.args = parser.parse_args()

    def init_problem(self):
        '''Initialize problem using user parameters and DB design retrieved from 
        SiQADConnector.'''

        # TODO the following loop gets DBs that are already in aggregates, see 
        # if there's a flag to prevent this or other ways of reading DBs such as 
        # directly accessing the items data structure.
        for db in self.sqconn.dbCollection():
            self.dbs.append((db.x, db.y))

        # plugin parameters
        self.d_inner_max = float(self.sqconn.getParameter('max_dbp_inner_distance'))
        self.d_inner_min = float(self.sqconn.getParameter('min_dbp_inner_distance'))
        print('inner_max={}, inner_min={}'.format(self.d_inner_max, self.d_inner_min))

    def perform_recognition(self):
        '''Contruct a recognition class, perform the recognition, and parse the 
        returned aggregate creation list.'''

        recognizer = DBPairRecognition(self.dbs, self.d_inner_min, self.d_inner_max)
        self.dbp_aggs = recognizer.run_recognition()

    def export_results(self):
        '''Convert the results stored in self.dbp_aggs to SiQADCommands and 
        export the commands through SiQADConnector.'''

        # convert to the following format:
        # [ [ (db_x1, db_y1), (db_x2, db_y2) ]
        #   [ (db_x3, db_y3), (db_x4, db_y4) ] ]
        dbp_aggs_with_loc = []
        for i in range(len(self.dbp_aggs)):
            agg = []
            for db_ind in self.dbp_aggs[i]:
                agg.append(self.dbs[db_ind])
            dbp_aggs_with_loc.append(agg)
        #print(dbp_aggs_with_loc)

        for i in range(len(self.dbp_aggs)):
            #print('About to add command for agg: {}'.format(dbp_aggs_with_loc[i]))
            self.sqconn.addCommand('add', 'Aggregate', dbagg=dbp_aggs_with_loc[i])
            #self.sqconn.addSQCommand(siqadconn.AggregateCommand(dbp_aggs))


class DBPairRecognition:
    '''Recognize DB-pairs in the given list of DBs.'''

    dbs = []        # list of tuples of floats containing DB locations (x, y)

    def __init__(self, dbs, d_inner_min, d_inner_max):
        self.dbs = np.asarray(dbs)
        self.d_inner_min = d_inner_min
        self.d_inner_max = d_inner_max

    def run_recognition(self):
        '''Perform the recognition on the list of DBs already stored here.
        
        Returns:
            A list of tuples. Each item in the list represents an aggregate that
            should be formed, and each tuple stores the DB indices that should 
            form an aggregate.

                e.g. [ (db1, db2),
                       (db3, db4) ]
        '''

        if len(self.dbs) < 2:
            return []

        # TODO set default values for d_inner_min and max if the input ones are 
        # None.

        # construct distance matrix and prune out-of-range elements,
        # "pruning" is done by setting the out-of-bound values to values greater
        # than the max bound
        db_r = distance.cdist(self.dbs, self.dbs, 'euclidean')
        db_r_pruned = np.where((db_r>self.d_inner_min) & (db_r<self.d_inner_max), 
                db_r, self.d_inner_max+1)

        # find DB index corresponding to the nearest DB of each row
        min_ind = np.argmin(db_r_pruned, axis=1)

        db_ind_aggs = []
        for i in range(len(min_ind)):
            if db_r_pruned[i][min_ind[i]] > self.d_inner_max or min_ind[i] < 0:
                continue
            j = min_ind[i]
            if i == j:
                continue
            if i == min_ind[j]:
                # found mutual nearest neighbor
                print('nearest neighbors (min_ind[i], min_ind[j])={}'.format((min_ind[i], min_ind[j])))
                db_ind_aggs.append((min_ind[i], min_ind[j]))
                min_ind[i] = -1
                min_ind[j] = -1
        return db_ind_aggs

if __name__ == "__main__":
    import sys
    interface = SiQADInterface()
    sys.exit(interface.exec())
