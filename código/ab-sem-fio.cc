#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/log.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/wave-module.h"
#include "ns3/wifi-module.h"

using namespace ns3;
using namespace std;

//     A (Cliente)     <----->     B (Servidor)
//   (posição: 0)                     (posição: 50)

int main(int argc, char *argv[]) {
  PacketMetadata::Enable();
  Time::SetResolution(Time::NS);
  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  NodeContainer nodes;
  nodes.Create(2);

  YansWifiChannelHelper wifChHelper = YansWifiChannelHelper::Default();
  Ptr<YansWifiChannel> wifiChannel = wifChHelper.Create();

  YansWifiPhyHelper wifiPhyLayer;
  wifiPhyLayer.SetChannel(wifiChannel);

  wifiPhyLayer.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11);
  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default();
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default();
  wifi80211p.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                                     StringValue("OfdmRate6MbpsBW10MHz"),
                                     "ControlMode",
                                     StringValue("OfdmRate6MbpsBW10MHz"));

  NetDeviceContainer netDevContainer =
      wifi80211p.Install(wifiPhyLayer, wifi80211pMac, nodes);

  InternetStackHelper internetStackHelper;
  internetStackHelper.Install(nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer a = ipv4.Assign(netDevContainer);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);

  Ptr<ConstantPositionMobilityModel> staticMod =
      nodes.Get(0)->GetObject<ConstantPositionMobilityModel>();
  staticMod->SetPosition(Vector(0, 0, 0));

  staticMod = nodes.Get(1)->GetObject<ConstantPositionMobilityModel>();
  staticMod->SetPosition(Vector(50, 0, 0));

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