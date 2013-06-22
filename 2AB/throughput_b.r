library(ggplot2)

pdf(file="ReNe_SoSe_2013_PA2b_943147_throughput.pdf", onefile=TRUE, pagecentre=TRUE, width=12, height=6)

file <- read.csv(file="./ReNe_SoSe_2013_PA2b_943147_throughput.csv", head=TRUE, sep=",")

print(
    qplot(time,bytes,data=file,geom="line",color=node, xlab="Time [s]", ylab="Throughput [kbps]", main="Throughput")

)
