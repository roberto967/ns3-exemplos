#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/log.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"

using namespace ns3;
using namespace std;

int main(int argc, char *argv[]) {
  PacketMetadata::Enable();
  Time::SetResolution(Time::NS);
  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  NodeContainer nodes;
  nodes.Create(2);

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211g);

  YansWifiChannelHelper wifChHelper = YansWifiChannelHelper::Default();
  Ptr<YansWifiChannel> wifiChannel = wifChHelper.Create();
  YansWifiPhyHelper wifiPhyLayer;
  wifiPhyLayer.SetChannel(wifiChannel);

  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");

  NetDeviceContainer devices = wifi.Install(wifiPhyLayer, wifiMac, nodes);

  InternetStackHelper internet;
  internet.Install(nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer a = ipv4.Assign(devices);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);

  Ptr<ConstantPositionMobilityModel> node0Mobility =
      nodes.Get(0)->GetObject<ConstantPositionMobilityModel>();
  node0Mobility->SetPosition(Vector(100, 100, 0));

  Ptr<ConstantPositionMobilityModel> node1Mobility =
      nodes.Get(1)->GetObject<ConstantPositionMobilityModel>();
  node1Mobility->SetPosition(Vector(150, 100, 0));

  UdpEchoServerHelper echoServer(9);

  ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(10.0));

  UdpEchoClientHelper echoClient(a.GetAddress(1), 9);
  echoClient.SetAttribute("MaxPackets", UintegerValue(5));
  echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  echoClient.SetAttribute("PacketSize", UintegerValue(1024));

  ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
  clientApps.Start(Seconds(2.0));
  clientApps.Stop(Seconds(10.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
