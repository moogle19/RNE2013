library(ggplot2)

pdf(file="ReNe_SoSe_2013_PA2a_943147_boxplot.pdf", onefile=TRUE, pagecentre=TRUE, width=30, height=15)

file <- read.csv(file="./ReNe_SoSe_2013_PA2a_943147_throughput.csv", head=TRUE, sep=",")

print(
    qplot(time,bytes,data=file,geom="boxplot",color=node, xlab="Time in s", ylab="Throughput in mb", main="Throughput")
)
