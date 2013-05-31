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

static const uint32_t totalTxBytes = 20000;
static uint32_t currentTxBytes = 0;
static const uint32_t writeSize = 1040;
uint8_t data[writeSize];

void StartFlow (Ptr<Socket>, Ipv4Address, uint16_t);
void WriteUntilBufferFull (Ptr<Socket>, uint32_t);

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
	*/
	
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

    AsciiTraceHelper ascii;
    ptp.EnableAsciiAll (ascii.CreateFileStream ("first.tr"));
    ptp.EnablePcapAll ("first");
	
	Simulator::Stop (Seconds (60));
	Simulator::Run ();
	Simulator::Destroy ();

	return 0;
}

 void StartFlow(Ptr<Socket> localSocket,
                  Ipv4Address servAddress,
                  uint16_t servPort)
   {
     NS_LOG_INFO ("Starting flow at time " <<  Simulator::Now ().GetSeconds ());
     currentTxBytes = 0;
     localSocket->Bind ();
     localSocket->Connect (InetSocketAddress (servAddress, servPort));//connect
   
     // tell the tcp implementation to call WriteUntilBufferFull again
     // if we blocked and new tx buffer space becomes available
     localSocket->SetSendCallback (MakeCallback (&WriteUntilBufferFull));
     WriteUntilBufferFull (localSocket, localSocket->GetTxAvailable ());
   }

void WriteUntilBufferFull (Ptr<Socket> localSocket, uint32_t txSpace)
{
	while (currentTxBytes < totalTxBytes && localSocket->GetTxAvailable () > 0)
	{
		uint32_t left = totalTxBytes - currentTxBytes;
		uint32_t dataOffset = currentTxBytes % writeSize;
		uint32_t toWrite = writeSize - dataOffset;
		toWrite = std::min (toWrite, left);
		toWrite = std::min (toWrite, localSocket->GetTxAvailable ());
		int amountSent = localSocket->Send (&data[dataOffset], toWrite, 0);
		if(amountSent < 0)
		{
			// we will be called again when new tx space becomes available.
			return;
		}
		currentTxBytes += amountSent;
	}
	localSocket->Close ();
}
