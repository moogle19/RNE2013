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

accu_outfile = ["./ReNe_SoSe_2013_PA2a_943147_accuthroughput.csv", "./ReNe_SoSe_2013_PA2b_943147_accuthroughput.csv"]
throughput_outfile = ["./ReNe_SoSe_2013_PA2a_943147_throughput.csv", "./ReNe_SoSe_2013_PA2b_943147_throughput.csv",  "./ReNe_SoSe_2013_PA2a_943147_throughput_stacked.csv", "./ReNe_SoSe_2013_PA2b_943147_throughput_stacked.csv"]

for z in range(4) :

    throughput = [0,0,0,0,0,0]
    accu = [0,0,0,0,0,0]

    timeval = 0.25
    time = 0

    outfile_accu = open(accu_outfile[z%2], 'wb')
    outfile_throughput = open(throughput_outfile[z], 'wb')
    writer_accu = csv.writer(outfile_accu, delimiter=',', quotechar='|', quoting=csv.QUOTE_MINIMAL)
    writer_throughput = csv.writer(outfile_throughput, delimiter=',', quotechar='|', quoting=csv.QUOTE_MINIMAL)

    writer_accu.writerow(["bytes", "time", "node"])
    writer_throughput.writerow(["bytes", "time", "node"])

    with open(infile[z%2]) as f :
        for line in f:
            match = regex.match(line)
            if match:
                if int(match.group("node")) < 5 :
                    if time == 0 :
                        time = math.floor(float(match.group("time")))
                    
                    if float(match.group("time")) < time+timeval :
                        size = (int(match.group("length")) + 2) * 8
                    
                        throughput[int(match.group("node"))] = throughput[int(match.group("node"))] + size
                        throughput[5] = throughput[5] + size
                        accu[int(match.group("node"))] = accu[int(match.group("node"))] + size
                        accu[5] = accu[5] + size
                    
                    else :
                        for i in range(5) :
                            writer_accu.writerow([accu[i] / (1000) , time, "node " + str(i)])
                            writer_throughput.writerow([throughput[i] / (1000 * timeval), time, "node " + str(i)])
                            throughput[i] = 0
                            writer_accu.writerow([accu[5] / (1000), time, "all nodes"])
                        if z < 2 :
                            writer_throughput.writerow([throughput[5] / (1000 * timeval), time, "all nodes"])
                        throughput[5] = 0
                        time = time + timeval
                        
    outfile_accu.close()
    outfile_throughput.close()
