#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <stdio.h>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/animation-interface.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ReNe_SoSe_2013_PA2a_943147");


int main(int argc, char** argv) {
	LogComponentEnable("ReNe_SoSe_2013_PA2a_943147", LOG_LEVEL_INFO);
	NS_LOG_INFO("START");

	//Create Nodes
	Ptr<Node> nClient0 = CreateObject<Node>();
	Ptr<Node> nClient1 = CreateObject<Node>();
	Ptr<Node> nClient2 = CreateObject<Node>();
	Ptr<Node> nClient3 = CreateObject<Node>();
	Ptr<Node> nClient4 = CreateObject<Node>();
	Ptr<Node> nRouter = CreateObject<Node>();
	Ptr<Node> nServer = CreateObject<Node>();
	
	//Container for all Nodes
	NodeContainer ncAll;
	ncAll.Add(nClient0);
	ncAll.Add(nClient1);
	ncAll.Add(nClient2);
	ncAll.Add(nClient3);
	ncAll.Add(nClient4);
	ncAll.Add(nRouter);
	ncAll.Add(nServer);
	
	//Install InternetStackHelper
	InternetStackHelper stack;
	stack.Install(ncAll);	
	
	//Create NodeContainer for all connections
	NodeContainer ncRouterClient0 = NodeContainer(nClient0, nRouter);
	NodeContainer ncRouterClient1 = NodeContainer(nClient1, nRouter);
	NodeContainer ncRouterClient2 = NodeContainer(nClient2, nRouter);
	NodeContainer ncRouterClient3 = NodeContainer(nClient3, nRouter);
	NodeContainer ncRouterClient4 = NodeContainer(nClient4, nRouter);
	NodeContainer ncServerRouter = NodeContainer(nServer, nRouter);
	
	//Create Point-to-Point-Helper with given DataRate and Delay
	PointToPointHelper ptp;
	ptp.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
	ptp.SetChannelAttribute("Delay", StringValue("20ms"));
	
	//Install Point-to-Point connections
	NetDeviceContainer RouterToC0 = ptp.Install(ncRouterClient0);
	NetDeviceContainer RouterToC1 = ptp.Install(ncRouterClient1);
	NetDeviceContainer RouterToC2 = ptp.Install(ncRouterClient2);
	NetDeviceContainer RouterToC3 = ptp.Install(ncRouterClient3);
	NetDeviceContainer RouterToC4 = ptp.Install(ncRouterClient4);
	NetDeviceContainer ServerToRouter = ptp.Install(ncServerRouter);
	
	//asign addresses to nodes
	Ipv4AddressHelper ipv4;
	ipv4.SetBase("10.0.0.0", "255.255.255.0");
	Ipv4InterfaceContainer iRouterToC0 = ipv4.Assign(RouterToC0);
	ipv4.SetBase("10.0.1.0", "255.255.255.0");
	Ipv4InterfaceContainer iRouterToC1 = ipv4.Assign(RouterToC1);
	ipv4.SetBase("10.0.2.0", "255.255.255.0");
	Ipv4InterfaceContainer iRouterToC2 = ipv4.Assign(RouterToC2);
	ipv4.SetBase("10.0.3.0", "255.255.255.0");
	Ipv4InterfaceContainer iRouterToC3 = ipv4.Assign(RouterToC3);
	ipv4.SetBase("10.0.4.0", "255.255.255.0");
	Ipv4InterfaceContainer iRouterToC4 = ipv4.Assign(RouterToC4);
	ipv4.SetBase("10.0.5.0", "255.255.255.0");
	Ipv4InterfaceContainer iServerToRouter = ipv4.Assign(ServerToRouter);
	
	//generate Routingtable for our network
	Ipv4GlobalRoutingHelper::PopulateRoutingTables();
	
	//enable Ascii Trace
	AsciiTraceHelper ascii;
    	ptp.EnableAsciiAll (ascii.CreateFileStream ("ReNe_SoSe_2013_PA2a_943147.tr"));
	
	//set portnumber
	uint16_t port = 50000;
	
	//Install OnOffHelper to Server for every node
	OnOffHelper onOffHelperC0 ("ns3::TcpSocketFactory", Address(InetSocketAddress (iRouterToC0.GetAddress(0), port)));
    	onOffHelperC0.SetConstantRate(DataRate("1Mbps"));
	ApplicationContainer sinkApps = onOffHelperC0.Install(nServer);
	sinkApps.Start(Seconds(5.0));
	sinkApps.Stop(Seconds(65.0));

	OnOffHelper onOffHelperC1 ("ns3::TcpSocketFactory", Address(InetSocketAddress (iRouterToC1.GetAddress(0), port)));
    	onOffHelperC1.SetConstantRate(DataRate("1Mbps"));
	sinkApps = onOffHelperC1.Install(nServer);
	sinkApps.Start(Seconds(10.0));
	sinkApps.Stop(Seconds(65.0));

	OnOffHelper onOffHelperC2 ("ns3::TcpSocketFactory", Address(InetSocketAddress (iRouterToC2.GetAddress(0), port)));
    	onOffHelperC2.SetConstantRate(DataRate("1Mbps"));
	sinkApps = onOffHelperC2.Install(nServer);
	sinkApps.Start(Seconds(15.0));
	sinkApps.Stop(Seconds(65.0));

	OnOffHelper onOffHelperC3 ("ns3::TcpSocketFactory", Address(InetSocketAddress (iRouterToC3.GetAddress(0), port)));
	onOffHelperC3.SetConstantRate(DataRate("1Mbps"));
	sinkApps = onOffHelperC3.Install(nServer);
	sinkApps.Start(Seconds(20.0));
	sinkApps.Stop(Seconds(65.0));

	OnOffHelper onOffHelperC4 ("ns3::TcpSocketFactory", Address(InetSocketAddress (iRouterToC4.GetAddress(0), port)));
    	onOffHelperC4.SetConstantRate(DataRate("1Mbps"));
	sinkApps = onOffHelperC4.Install(nServer);
	sinkApps.Start(Seconds(25.0));
	sinkApps.Stop(Seconds(65.0));


	//Install PacketSinkHelper to every node
    	PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
	sinkApps = sinkHelper.Install(nClient0); 
	sinkApps.Start(Seconds(5.0));
	sinkApps.Stop(Seconds(65.0));

	sinkApps = sinkHelper.Install(nClient1); 
	sinkApps.Start(Seconds(10.0));
	sinkApps.Stop(Seconds(65.0));

	sinkApps = sinkHelper.Install(nClient2); 
	sinkApps.Start(Seconds(15.0));
	sinkApps.Stop(Seconds(65.0));

	sinkApps = sinkHelper.Install(nClient3); 
	sinkApps.Start(Seconds(20.0));
	sinkApps.Stop(Seconds(65.0));

	sinkApps = sinkHelper.Install(nClient4); 
	sinkApps.Start(Seconds(25.0));
	sinkApps.Stop(Seconds(65.0));
	
	sinkApps = sinkHelper.Install(nRouter);
	sinkApps.Start(Seconds(5.0));
	sinkApps.Stop(Seconds(65.0));

	//enable pcap output
    	ptp.EnablePcapAll ("ReNe_SoSe_2013_PA2a_943147_");

	//enable xml-file for NetAnim
	AnimationInterface anim ("ReNe_SoSe_2013_PA2a_943147.xml");
	anim.SetConstantPosition (nServer, 600, 300);
	anim.SetConstantPosition (nRouter, 300, 300);
	anim.SetConstantPosition (nClient0, 100, 100);
	anim.SetConstantPosition (nClient1, 100, 200);
	anim.SetConstantPosition (nClient2, 100, 300);
	anim.SetConstantPosition (nClient3, 100, 400);
	anim.SetConstantPosition (nClient4, 100, 500);


	NS_LOG_INFO("RUN");
	//stop simulator after 70 Seconds
	Simulator::Stop (Seconds (70));
	//run simulator
	Simulator::Run ();
	
	Simulator::Destroy ();

	return 0;
}

