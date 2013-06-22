library(ggplot2)

pdf(file="ReNe_SoSe_2013_PA2a_943147_stackedbar.pdf", onefile=TRUE, pagecentre=TRUE, width=12, height=6)

file <- read.csv(file="./ReNe_SoSe_2013_PA2a_943147_throughput_stacked.csv", head=TRUE, sep=",")
time_in_s <- file$time
throughput_in_kbps <- file$bytes

print(
	ggplot(file, aes(x=time_in_s, y=throughput_in_kbps, fill=node)) + 
	  geom_bar(stat="identity")
)