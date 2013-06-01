#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");


int main(int argc, char** argv) {
	Ptr<Node> nClient0 = CreateObject<Node>();
	Ptr<Node> nClient1 = CreateObject<Node>();
	Ptr<Node> nClient2 = CreateObject<Node>();
	Ptr<Node> nClient3 = CreateObject<Node>();
	Ptr<Node> nClient4 = CreateObject<Node>();
	Ptr<Node> nServer = CreateObject<Node>();
	Ptr<Node> nRouter = CreateObject<Node>();
	
	NodeContainer ncAll;
	ncAll.Add(nClient0);
	ncAll.Add(nClient1);
	ncAll.Add(nClient2);
	ncAll.Add(nClient3);
	ncAll.Add(nClient4);
	ncAll.Add(nServer);
	ncAll.Add(nRouter);

	InternetStackHelper stack;
	stack.Install(ncAll);	

	NodeContainer ncRouterClient0 = NodeContainer(nClient0, nRouter);
	NodeContainer ncRouterClient1 = NodeContainer(nClient1, nRouter);
	NodeContainer ncRouterClient2 = NodeContainer(nClient2, nRouter);
	NodeContainer ncRouterClient3 = NodeContainer(nClient3, nRouter);
	NodeContainer ncRouterClient4 = NodeContainer(nClient4, nRouter);
	NodeContainer ncServerRouter = NodeContainer(nServer, nRouter);
	
	PointToPointHelper ptp;
	ptp.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
	ptp.SetChannelAttribute("Delay", StringValue("20ms"));
	
	NetDeviceContainer ServerToRouter = ptp.Install(ncServerRouter);
	NetDeviceContainer RouterToC0 = ptp.Install(ncRouterClient0);
	NetDeviceContainer RouterToC1 = ptp.Install(ncRouterClient1);
	NetDeviceContainer RouterToC2 = ptp.Install(ncRouterClient2);
	NetDeviceContainer RouterToC3 = ptp.Install(ncRouterClient3);
	NetDeviceContainer RouterToC4 = ptp.Install(ncRouterClient4);

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
	
	/*
	Ptr<Socket> C0Sock = Socket::CreateSocket(nClient0, TypeId::LookupByName ("ns3::TcpSocketFactory"));
	Ptr<Socket> C1Sock = Socket::CreateSocket(nClient1, TypeId::LookupByName ("ns3::TcpSocketFactory"));
	Ptr<Socket> C2Sock = Socket::CreateSocket(nClient2, TypeId::LookupByName ("ns3::TcpSocketFactory"));
	Ptr<Socket> C3Sock = Socket::CreateSocket(nClient3, TypeId::LookupByName ("ns3::TcpSocketFactory"));
	Ptr<Socket> C4Sock = Socket::CreateSocket(nClient4, TypeId::LookupByName ("ns3::TcpSocketFactory"));
	
    
	uint16_t port = 50000;
  	Address sinkLocalAddress(InetSocketAddress (Ipv4Address::GetAny (), port));
  	PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
  	ApplicationContainer sinkApp = sinkHelper.Install (nServer);
  	sinkApp.Start (Seconds (1.0));
  	sinkApp.Stop (Seconds (10.0));
	
	UdpEchoServerHelper echoServer (42);
	ApplicationContainer serverApps = echoServer.Install (ncRouterClient0.Get (1));
	serverApps.Start (Seconds (1.0));
	serverApps.Stop (Seconds (10.0));

	UdpEchoClientHelper echoClient (iRouterToC0.GetAddress(1), 42);
	echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
	echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
	echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

	ApplicationContainer clientApps = echoClient.Install (ncRouterClient0.Get(0));
	clientApps.Start (Seconds (2.0));
	clientApps.Stop (Seconds (10.0));
	*/

//Turn on global static routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
   
    // Create a packet sink on the star "hub" to receive these packets
    uint16_t port = 50000;
    Address sinkLocalAddress (InetSocketAddress (iRouterToC0.GetAddress(0), port));
    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp = sinkHelper.Install (nClient0);
    sinkApp.Start (Seconds (0.));
    sinkApp.Stop (Seconds (100.));

	Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nServer, TcpSocketFactory::GetTypeId ()); //source at Server
	
    Address sinkLocalAddress1 (InetSocketAddress (iRouterToC1.GetAddress(0), port));
    PacketSinkHelper sinkHelper1 ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp1 = sinkHelper.Install (nClient1);
    sinkApp1.Start (Seconds (5.));
    sinkApp1.Stop (Seconds (100.));
    
	Address sinkLocalAddress2 (InetSocketAddress (iRouterToC2.GetAddress(0), port));
    PacketSinkHelper sinkHelper2 ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp2 = sinkHelper.Install (nClient2);
    sinkApp2.Start (Seconds (10.));
    sinkApp2.Stop (Seconds (100.));

	/*Ptr<MyApp> app = CreateObject<MyApp> ();
  	app->Setup (ns3TcpSocket, sinkLocalAddress, 1040, 100000, DataRate ("250Kbps"));
  	nRouter->AddApplication (app);
  	app->SetStartTime (Seconds (1.));
  	app->SetStopTime (Seconds (100.));
*/
	// Create the OnOff applications to send TCP to the server
    OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address ());
    clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    
	OnOffHelper clientHelper1 ("ns3::TcpSocketFactory", Address ());
    clientHelper1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    clientHelper1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    
	OnOffHelper clientHelper2 ("ns3::TcpSocketFactory", Address ());
    clientHelper2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    clientHelper2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
   
    //normally wouldn't need a loop here but the server IP address is different
    //on each p2p subnet
    ApplicationContainer clientApps;
    AddressValue remoteAddress(InetSocketAddress (iRouterToC0.GetAddress (0), port));
    clientHelper.SetAttribute ("Remote", remoteAddress);
    clientApps.Add (clientHelper.Install (nServer));
    clientApps.Start (Seconds (1.0));
    clientApps.Stop (Seconds (100.0));

    ApplicationContainer clientApps1;
    AddressValue remoteAddress1(InetSocketAddress (iRouterToC1.GetAddress (0), port));
    clientHelper1.SetAttribute ("Remote", remoteAddress1);
    clientApps1.Add (clientHelper1.Install (nServer));
    clientApps1.Start (Seconds (5.0));
    clientApps1.Stop (Seconds (100.0));
    
	ApplicationContainer clientApps2;
    AddressValue remoteAddress2(InetSocketAddress (iRouterToC2.GetAddress (0), port));
    clientHelper2.SetAttribute ("Remote", remoteAddress2);
    clientApps2.Add (clientHelper2.Install (nServer));
    clientApps2.Start (Seconds (10.0));
    clientApps2.Stop (Seconds (100.0));

    AsciiTraceHelper ascii;
    ptp.EnableAsciiAll (ascii.CreateFileStream ("first.tr"));
    ptp.EnablePcapAll ("first");
	
	Simulator::Stop (Seconds (100));
	Simulator::Run ();
	Simulator::Destroy ();

	return 0;
}

