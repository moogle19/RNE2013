library(ggplot2)

pdf(file="ReNe_SoSe_2013_PA2a_943147_stackedbar.pdf", onefile=TRUE, pagecentre=TRUE, width=12, height=6)

file <- read.csv(file="./ReNe_SoSe_2013_PA2a_943147_throughput_stacked.csv", head=TRUE, sep=",")

print(
	ggplot(data = file, aes(x=time, y=bytes, fill=node)) + 
	  geom_bar(stat="identity") + xlab("Time [s]") + ylab("Throughput [kbps]") + ggtitle("A: Throughput")
)