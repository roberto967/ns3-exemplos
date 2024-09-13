// Comunicação entre três nós A, B e C, em que o nó A é o cliente,
// o nó C é o servidor e o nó B é a ponte/caminho entre os nós A e C,
// além disso, a comunicação entre os nós A e B é através de canal
// de comunicação ponto-a-ponto e a comunicação entre os nós B e C
// é através de canal de comunicação sem fio com uso da aplicação
// UDPEcho e envio de mensagem com 5 pacotes.

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("Task2"); /* Essa linha define o componente de log chamado “Task2”. 
Isso permite que você registre mensagens de log específicas para esse componente.*/

int main(int argc, char *argv[])
{
    
    //Essas linhas criam um objeto CommandLine e analisam os argumentos 
    //de linha de comando passados para o programa.
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS); //Define a resolução do tempo para nanossegundos.
    
    /*Habilita o log para as classes UdpEchoClientApplication e UdpEchoServerApplication com nível de log INFO.*/
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    //Cria um contêiner de nós e adiciona 3 nós a ele
    NodeContainer nodes; 
    nodes.Create(3);

    /*Configura a mobilidade dos nós com uma posição constante. Necessário para nós wifi*/
    MobilityHelper mobility; 
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0.0, 0.0, 0.0));
    positionAlloc->Add(Vector(10.0, 10.0, 0.0));
    positionAlloc->Add(Vector(20.0, 10.0, 0.0));
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    /*Configura um canal ponto a ponto entre os nós 0 e 1 e cria um contêiner de dispositivos de rede para essa conexão.*/
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer abDevice;
    abDevice = pointToPoint.Install(nodes.Get(0), nodes.Get(1));

//Configura um canal WiFi usando o modelo Yans e cria um canal WiFi.
    YansWifiChannelHelper wifiChannelHelper = YansWifiChannelHelper::Default();
    Ptr<YansWifiChannel> wifiChannel = wifiChannelHelper.Create();

/*Configura a camada física WiFi e define o gerenciador de estações remotas como “ns3::AarfWifiManager”*/    
    YansWifiPhyHelper wifiPhyLayer;
    wifiPhyLayer.SetChannel(wifiChannel);
    WifiHelper wifiHelper;
    wifiHelper.SetRemoteStationManager("ns3::AarfWifiManager");

/*Configura a camada de controle de acesso ao meio (MAC) WiFi para o nó 1 como uma estação (STA) com o SSID “ns-3-ssid1”.*/
    WifiMacHelper wifiMacHelper;
    Ssid ssid = Ssid("ns-3-ssid1");
    wifiMacHelper.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));

/*Cria um contêiner de dispositivos de rede para a conexão WiFi entre os nós 1 e 2.*/
    NetDeviceContainer bcDevice;
    bcDevice.Add(wifiHelper.Install(wifiPhyLayer, wifiMacHelper, nodes.Get(1)));
    wifiMacHelper.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    bcDevice.Add(wifiHelper.Install(wifiPhyLayer, wifiMacHelper, nodes.Get(2)));;

    //Instala a pilha de protocolos da Internet nos nós.
    InternetStackHelper stack;
    stack.Install(nodes);

    /*Configura os endereços IPv4 para os dispositivos de rede e atribui esses endereços aos contêineres de interfaces.*/
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interface = address.Assign(abDevice), interface2 = address.Assign(bcDevice);

    //configura um servidor de eco UDP na porta 9 e o instala no nó 2
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(2));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    /*Configura um cliente de eco UDP para enviar pacotes ao endereço IP do nó 2 na porta 9 a partir do nó 0.*/
    UdpEchoClientHelper echoClient(interface2.GetAddress(1), 9);
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
