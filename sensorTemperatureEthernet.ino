/* Biblioteca relacionada para poder usar o Ethernet */
#include <Ethernet.h>

/* Biblioteca relacionada para poder usar o DHT11 */
#include <DHT.h>

/* Biblioteca relacionada para poder usar o DS1302 */
#include  <virtuabotixRTC.h>

/* Definindo a porta */
#define port 80

/* DHT11 na entrada analógica e modelo */
#define DHTPIN A1
#define DHTTYPE DHT11

/* Definindo o valores para o DS1302 */
#define DS_CLK 6
#define DS_DAT 7
#define DS_RST 8

/* Definindo o endereço MAC e o endereço IP para o controlador */
/* Definindo o endereço IP que varia de acordo com a rede */
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(172, 18, 11, 219);

/* Definindo a porta do servidor */
EthernetServer server(port);

/* Inicializa o DHT11 */
DHT dht(DHTPIN, DHTTYPE);

/* Inicializa o DS1302 */
virtuabotixRTC ds1302(DS_CLK, DS_DAT, DS_RST);

void setup() {
  /* Abre a comunicação serial */
  Serial.begin(9600);
  while (!Serial) {
    ; /* Espera a porta serial se conectar, nativamente somente na porta USB */
  }
  
  /* Inicializa Ethernet e o servidor */
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  
  /* Iniciliza os DS1302 com valores */
  /** 
   *  1ª paramêtro -> segundos
   *  2ª paramêtro -> minutos
   *  3ª paramêtro -> horas
   *  4ª paramêtro -> dia de semana
   *  5ª paramêtro -> dia do mês
   *  6ª paramêtro -> mês
   *  7ª paramêtro -> ano
  **/
  ds1302.setDS1302Time(00, 47, 9, 6, 23, 3, 2018); 
}


void loop() {
  ds1302.updateTime();
  /* Fica esperando os clientes */
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    /* Requisição HTTP termina com uma linha em branco */
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        /* Se você chegou ao final da linha (recebeu um novo caractere) e a linha está em branco, a solicitação HTTP terminou, então pode enviar a resposta */
        if (c == '\n' && currentLineIsBlank) {
          /* Envia o cabeçalho de resposta */
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json;charset=UTF-8");
          client.println("Connection: close");  /* Conexão fecha depois de enviar a resposta */
          client.println("Refresh: 5");  /* Conexão fecha depois de enviar a resposta */
          client.println();
          
          float humidity = dht.readHumidity();
          float temperature = dht.readTemperature();

          /* Verifica se o valor de retorno é válido */
          if (isnan(temperature) || isnan(humidity)) {
            temperature = -1;
            humidity = -1;
            //client.println("{\"temperature\" : \"-1\", \"humidity\" : \"-1\", \"date\" : \"" + String(ds1302.dayofmonth) + "/" + String(ds1302.month) + "/" + String(ds1302.year) + "\"", \"hours\" : \"" + String(ds1302.hours) + ":" + String(ds1302.minutes) + ":" + String(ds1302.seconds) + "\"}");
          }
          
          String json = "{\"temperature\" : \"" + String(temperature) + "\",";
          json = json + "\"humidity\" : \"" + String(humidity) + "\",";
          json = json + "\"date\" : \"" + String(ds1302.dayofmonth) + "/" + String(ds1302.month) + "/" + String(ds1302.year) + "\",";
          json = json + "\"hours\" : \"" + String(ds1302.hours) + ":" + String(ds1302.minutes) + ":" + String(ds1302.seconds) + "\"}";

          client.println(json);
          /*else {
            //client.println("{\"temperature\" : \"" + String(temperature) + "\", \"humidity\" : \"" + String(humidity) + "\", \"date\" : \"" + String(ds1302.dayofmonth) + "/" + String(ds1302.month)  + "/" + String(ds1302.year) + "\"", \"hours\" : \"" + String(ds1302.hours) + ":" + String(ds1302.minutes) + ":" + String(ds1302.seconds) + "\"}");
          }*/
          
          break;
        }
        if (c == '\n') {
          /* Começa uma nova linha */
          currentLineIsBlank = true;
        } else if (c != '\r') {
          /* Se tem um caractere na linha atual */
          currentLineIsBlank = false;
        }
      }
    }
    /* Dá um tempo para o navegador receber os dados */
    delay(1);
    /* Fecha a conexão */
    client.stop();
    Serial.println("client disconnected");
  }
}

