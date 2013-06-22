library(ggplot2)

pdf(file="ReNe_SoSe_2013_PA2a_943147_accu.pdf", onefile=TRUE, pagecentre=TRUE, width=30, height=15)

file <- read.csv(file="./ReNe_SoSe_2013_PA2a_943147_accuthroughput.csv", head=TRUE, sep=",")

print(
  qplot(time,bytes,data=file,geom="line",color=node, xlab="Time in s", ylab="Throughput in mbit/s", main="Accumulated throughput")
)