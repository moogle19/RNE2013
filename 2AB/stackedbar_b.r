library(ggplot2)

pdf(file="ReNe_SoSe_2013_PA2b_943147_stackedbar.pdf", onefile=TRUE, pagecentre=TRUE, width=30, height=15)

file <- read.csv(file="./ReNe_SoSe_2013_PA2b_943147_accuthroughput_stacked.csv", head=TRUE, sep=",")
time_in_s <- file$time
throughput_in_mbit <- file$bytes

print(
	ggplot(file, aes(x=time_in_s, y=throughput_in_mbit, fill=node)) + 
	  geom_bar(stat="identity")
)