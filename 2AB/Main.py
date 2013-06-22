'''
Created on 21.06.2013

@author: Kevin Seidel
'''
import sys, re, math
import csv

regex = re.compile(
r"""r+\s+
(?P<time>.*?)\s+
/NodeList/(?P<node>.*?)/DeviceList+(?P<rest>.*?)\s+
length: \s+
(?P<length>.*?)\s+
(?P<sendaddr>.*?)\s+ > \s+
(?P<recaddr>.*?)\s+
"""
, re.VERBOSE)

infile = ["./ReNe_SoSe_2013_PA2a_943147.tr", "./ReNe_SoSe_2013_PA2b_943147.tr"]

accu_outfile = ["./ReNe_SoSe_2013_PA2a_943147_accuthroughput.csv", "./ReNe_SoSe_2013_PA2b_943147_accuthroughput.csv", "./ReNe_SoSe_2013_PA2a_943147_accuthroughput_stacked.csv", "./ReNe_SoSe_2013_PA2b_943147_accuthroughput_stacked.csv"]
throughput_outfile = ["./ReNe_SoSe_2013_PA2a_943147_throughput.csv", "./ReNe_SoSe_2013_PA2b_943147_throughput.csv"]

for z in range(4) :

    accu = [0,0,0,0,0,0]
    summe = [0,0,0,0,0,0]

    timeval = 0.25
    time = 0

    outfile_accu = open(accu_outfile[z], 'wb')
    outfile_throughput = open(throughput_outfile[z%2], 'wb')
    writer_accu = csv.writer(outfile_accu, delimiter=',', quotechar='|', quoting=csv.QUOTE_MINIMAL)
    writer_throughput = csv.writer(outfile_throughput, delimiter=',', quotechar='|', quoting=csv.QUOTE_MINIMAL)

    writer_accu.writerow(["bytes", "time", "node"])
    writer_throughput.writerow(["bytes", "time", "node"])

    with open(infile[z%2]) as f :
        for line in f:
            match = regex.match(line)
            if match:
                if int(match.group("node")) < 5 :
                    while float(match.group("time")) >= (time-timeval) :
                        for i in range(5) :
                            writer_accu.writerow([accu[i] / 1024, time, "node " + str(i)])
                            writer_throughput.writerow([summe[i], time, "node " + str(i)])
                            accu[i] = 0
                        if z < 2 :
                            writer_accu.writerow([accu[5] / 1024, time, "all nodes"])
                            writer_throughput.writerow([summe[5], time, "all nodes"])
                        accu[5] = 0
                        time = time + timeval
                
                    size = (int(match.group("length")) + 2) * 0.008
                
                    accu[int(match.group("node"))] = accu[int(match.group("node"))] + size / timeval
                    accu[5] = accu[5] + size / timeval
                    summe[int(match.group("node"))] = summe[int(match.group("node"))] + size
                    summe[5] = summe[5] + size

    outfile_accu.close()
    outfile_throughput.close()
