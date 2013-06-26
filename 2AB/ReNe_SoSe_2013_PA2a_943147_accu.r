library(ggplot2)

pdf(file="ReNe_SoSe_2013_PA2a_943147_accu.pdf", onefile=TRUE, pagecentre=TRUE, width=12, height=6)

file <- read.csv(file="./ReNe_SoSe_2013_PA2a_943147_accuthroughput.csv", head=TRUE, sep=",")

print(
	ggplot(data = file, aes(x=time, y=bytes)) + geom_line(aes(colour=node)) + ylab("Accumulated Throughput [kb]") + xlab("Time [s]") + ggtitle("A: Accumulated Throughput")
)