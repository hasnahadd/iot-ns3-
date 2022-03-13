#include "ns3/core-module.h" 
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/netanim-module.h"
#include "ns3/sixlowpan-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TEST");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  
  uint32_t nWifi = 4;
uint32_t nAP = 1;
  bool tracing = false;

  CommandLine cmd (__FILE__);
  //cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);


  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

   NodeContainer wifiApNode;
    wifiApNode.Create(nAP);
   NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);

YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy ;
  phy.SetChannel (channel.Create ());

WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");


 WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

 MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (4),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiStaNodes);



MobilityHelper ap_mobility;
Ptr<ListPositionAllocator> ap_positionAlloc = CreateObject<ListPositionAllocator> ();
ap_positionAlloc->Add (Vector (0.0, 0.0, 0.0));
ap_mobility.SetPositionAllocator (ap_positionAlloc);
ap_mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
ap_mobility.Install (wifiApNode);  


 InternetStackHelper stack;
//addrasse net device 
stack.Install (wifiStaNodes);
stack.Install (wifiApNode);



SixLowPanHelper sixlowpan;
sixlowpan.SetDeviceAttribute ("ForceEtherType", BooleanValue (true) );
NetDeviceContainer six1 = sixlowpan.Install (staDevices);

Ipv6AddressHelper ipv6;
ipv6.SetBase (Ipv6Address ("2001:1::"), Ipv6Prefix (64));
Ipv6InterfaceContainer i1 = ipv6.Assign (apDevices);

i1.SetForwarding (0, true);

i1.SetDefaultRouteInAllNodes (0);

std::cout << "hello" << std::endl;

ipv6.SetBase (Ipv6Address ("2001:2::"), Ipv6Prefix (64));
Ipv6InterfaceContainer i2 = ipv6.Assign (staDevices);
i2.SetForwarding (0, true);
i2.SetDefaultRouteInAllNodes (0);



//traffiquecontainer

  std::cout << i2.GetAddress (0, 1) << std::endl;

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (wifiStaNodes.Get(0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (i2.GetAddress(0,1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (wifiStaNodes.Get (nWifi - 1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

Simulator::Stop (Seconds (10.0));
phy.EnablePcap ("wifi",apDevices.Get(0));






AnimationInterface anim("TEST.xml");
anim.SetConstantPosition(wifiStaNodes.Get(0), 1.0, 2.0);
anim.SetConstantPosition(wifiStaNodes.Get(1), 2.0, 3.0);
anim.SetConstantPosition(wifiStaNodes.Get(2), 3.0, 4.0);
anim.SetConstantPosition(wifiStaNodes.Get(3), 4.0, 5.0);
anim.SetConstantPosition(wifiApNode.Get(5), 6.0, 6.0);
Simulator::Run ();
Simulator::Destroy ();
return 0;

}
