library(ggplot2)

pdf(file="ReNe_SoSe_2013_PA2b_943147_throughput.pdf", onefile=TRUE, pagecentre=TRUE, width=12, height=6)

file <- read.csv(file="./ReNe_SoSe_2013_PA2b_943147_throughput.csv", head=TRUE, sep=",")

print(
	ggplot(data = file, aes(x=time, y=bytes)) + geom_line(aes(colour=node)) + ylab("Throughput [kbps]") + xlab("Time [s]") + ggtitle("B: Throughput")
)
