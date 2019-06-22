#!/usr/bin/python

import csv

with open('../mapFile/Sheet 1-Graph Info.csv') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=';')
    line_count = 0
    for row in csv_reader:
        print('\t{} | {}'.format(row[0], row[1]))
        line_count += 1
    print('Processed {} lines.'.format(line_count))