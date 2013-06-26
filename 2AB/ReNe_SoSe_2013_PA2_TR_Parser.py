'''
Created on 21.06.2013

@author: Kevin Seidel
'''
import sys, re, math
import csv

#regex for parsing tr-file
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

#inputfile for Simulation A and Simulation B
infile = ["./ReNe_SoSe_2013_PA2a_943147.tr", "./ReNe_SoSe_2013_PA2b_943147.tr"]

#Output for accumulated throughput
accu_outfile = ["./ReNe_SoSe_2013_PA2a_943147_accuthroughput.csv", "./ReNe_SoSe_2013_PA2b_943147_accuthroughput.csv"]
#Output for throughput with whole throughhput and without whole throughput (for stacked bar chart)
throughput_outfile = ["./ReNe_SoSe_2013_PA2a_943147_throughput.csv", "./ReNe_SoSe_2013_PA2b_943147_throughput.csv",  "./ReNe_SoSe_2013_PA2a_943147_throughput_stacked.csv", "./ReNe_SoSe_2013_PA2b_943147_throughput_stacked.csv"]

#do 4 times (1 time for every outputfile)
for z in range(4) :

    #throughput lists
    throughput = [0,0,0,0,0,0]
    accu = [0,0,0,0,0,0]
    
    #time interval for output
    timeval = 0.25
    time = 0
    
    #open output files
    outfile_accu = open(accu_outfile[z%2], 'wb')
    outfile_throughput = open(throughput_outfile[z], 'wb')
    
    #create csv writers for output
    writer_accu = csv.writer(outfile_accu, delimiter=',', quotechar='|', quoting=csv.QUOTE_MINIMAL)
    writer_throughput = csv.writer(outfile_throughput, delimiter=',', quotechar='|', quoting=csv.QUOTE_MINIMAL)

    #write header
    writer_accu.writerow(["bytes", "time", "node"])
    writer_throughput.writerow(["bytes", "time", "node"])

    with open(infile[z%2]) as f :
        for line in f: #read linewise
            match = regex.match(line) #use regex on line
            if match:
                if int(match.group("node")) < 5 : #only look for node 0 - 4
                    if time == 0 :
                        time = math.floor(float(match.group("time"))) #set time to first second in trace file
                    
                    if float(match.group("time")) < time+timeval : #check if time is in current timeval
                        size = (int(match.group("length")) + 2) * 8
                        
                        #add size to countervariables
                        throughput[int(match.group("node"))] = throughput[int(match.group("node"))] + size
                        throughput[5] = throughput[5] + size
                        accu[int(match.group("node"))] = accu[int(match.group("node"))] + size
                        accu[5] = accu[5] + size
                    
                    else :
                        #if time is out of timeval
                        for i in range(5) :
                            #write values to csv file linewise
                            writer_accu.writerow([accu[i] / (1000) , time, "node " + str(i)])
                            writer_throughput.writerow([throughput[i] / (1000 * timeval), time, "node " + str(i)])
                            throughput[i] = 0 #reset throughput
                        writer_accu.writerow([accu[5] / (1000), time, "all nodes"])
                        if z < 2 :
                            writer_throughput.writerow([throughput[5] / (1000 * timeval), time, "all nodes"])
                        throughput[5] = 0
                        time = time + timeval #increase timeval
    
    #close outputfiles        
    outfile_accu.close()
    outfile_throughput.close()
