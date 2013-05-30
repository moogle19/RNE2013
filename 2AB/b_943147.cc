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
	
	NodeContainer ncAll = NodeContainer(nClient0, nClient1, nClient2, nClient3, nClient4, nRouter, nServer);
	InternetStackHelper stack;
	stack.Install(ncAll);	

	NodeContainer ncServerRouter = NodeContainer(nServer, nRouter);
	NodeContainer ncRouterClient0 = NodeContainer(nRouter, nClient0);
	NodeContainer ncRouterClient1 = NodeContainer(nRouter, nClient1);
	NodeContainer ncRouterClient2 = NodeContainer(nRouter, nClient2);
	NodeContainer ncRouterClient3 = NodeContainer(nRouter, nClient3);
	NodeContainer ncRouterClient4 = NodeContainer(nRouter, nCLient4);
	
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
	ipv4.SetBase("10.0.0.1", "255.255.255.0");
	Ipv4InterfaceContainer iRouterToC0 = ipv4.Assign(RouterToC0);
	ipv4.SetBase("10.0.1.1", "255.255.255.0");
	Ipv4InterfaceContainer iRouterToC1 = ipv4.Assign(RouterToC1);
	ipv4.SetBase("10.0.2.1", "255.255.255.0");
	Ipv4InterfaceContainer iRouterToC2 = ipv4.Assign(RouterToC2);
	ipv4.SetBase("10.0.3.1", "255.255.255.0");
	Ipv4InterfaceContainer iRouterToC3 = ipv4.Assign(RouterToC3);
	ipv4.SetBase("10.0.4.1", "255.255.255.0");
	Ipv4InterfaceContainer iRouterToC4 = ipv4.Assign(RouterToC4);
	ipv4.SetBase("10.0.5.1", "255.255.255.0");
	Ipv4InterfaceContainer iServerToRouter = ipv4.Assign(ServerToRouter);

	Ptr<Socket> C0Sock = Socket::CreateSocket(nClient0, TypeId::LookupByName ("ns3::TcpSocketFactory"));
	Ptr<Socket> C1Sock = Socket::CreateSocket(nClient1, TypeId::LookupByName ("ns3::TcpSocketFactory"));
	Ptr<Socket> C2Sock = Socket::CreateSocket(nClient2, TypeId::LookupByName ("ns3::TcpSocketFactory"));
	Ptr<Socket> C3Sock = Socket::CreateSocket(nClient3, TypeId::LookupByName ("ns3::TcpSocketFactory"));
	Ptr<Socket> C4Sock = Socket::CreateSocket(nClient4, TypeId::LookupByName ("ns3::TcpSocketFactory"));

}
