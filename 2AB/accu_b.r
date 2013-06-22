library(ggplot2)

pdf(file="ReNe_SoSe_2013_PA2b_943147_accu.pdf", onefile=TRUE, pagecentre=TRUE, width=12, height=6)

file <- read.csv(file="./ReNe_SoSe_2013_PA2b_943147_accuthroughput.csv", head=TRUE, sep=",")

print(
  qplot(time,bytes,data=file,geom="line",color=node, xlab="Time [s]", ylab="Accumulated Throughput [kbit]", main="Accumulated throughput")
)