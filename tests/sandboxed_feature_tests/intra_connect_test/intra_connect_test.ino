/*
CHANGES LOG

1. the program could not connect to the server, and I added client.setInsecure() 
to skip the certificate check. It helped, the server responded, but with 
an Invalid Input error.

2. I noticed an error in the construction of the Server Request for obtaining
a TOKEN and corrected it. The original (incorrect) version: 

String auth_request = "client_id=";
auth_request += UID;
auth_request += "&client_secret=";
auth_request += SECRET;
auth_request += "&grant_type=client_credentials";

The server responded with the same error Invalid input (Invalid request)

3. I added https://api.intra.42.fr before the HTTPS request for TOKEN, 
as found in RandomNerdTutorials The server responded with the same error 
Invalid input (Invalid request) .
But is it a server error? Maybe it's an ArduinoJson error?????

4. At this point I'm pretty sure it's an ArduinoJson error. But before I
start figuring it out, I'll try using the advice I just saw:
use HTTP/1.0 instead of HTTP/1.1 in your request — It didn't help, 
the program started crashing with an unknown error, I'm reverting the changes back

5. Tried replacing ContentType in the application/x-www-form-urlencoded statement
in the request with ContentType in the application/vnd.api+json explanation, 
as suggested by 42 API The server responded with a 401 UnauthorizedServer error 42 API", error=invalid request 

6. Tried putting "?" in the request "?grant_type =client_credentials&client_id=grant_type=client_credentials&client_id="
The server responded with a 401 UnauthorizedServer error 42 API", error=invalid request 

7. I suspect that the program doesn't read the server message completely because of this function:
String server_response = client.readStringUntil('\n');
I'm trying to change it to String server_response = client.readString();
My suspicions were justified! After making changes to Serial Monitoring, an access token finally appeared!
But now the microprocessor has started to depend - perhaps due to the large amount of information.

8. It was noticed that in the server response the end of the message is marked with the '}' sign.
Now I'm trying to change the line server_response = client.readString() to the line
server_response = client.readStringUntil('}');
The server message has indeed started to be read up to the } symbol, but not including it.
I think that this is why I get the error Error parsing server response: EmptyInput

9. I'll try this part of the code while (client.connected()) { String server_response = client.readStringUntil('}');
if (server_response == "\r") {break;
} Serial.print(server_response);
} Serial.println("\n");
rewrite it to get the whole server message in the server_response variable and then extract the TOKEN from it.
It worked! Here is the part of the code that creates
String server_response = client.readStringUntil('}');
Serial.println(server_response);
int i = server_response.indexOf("{\"access_token\":\"");
String token = server_response.substring(i + 17, i + 81);
Serial.println("\n");
Serial.println( token);
Serial.println("\n");
server_response.clear();

10. COMPLETION! The entire program, from turning on and connecting to Wi-Fi to issuing the exam time and closing,
takes 3.5 seconds! All that remains is to polish the program a little, bring it to the project code, test it and
transfer it to the main program file.

*/


#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

int rtc_g_year = 2024;
int rtc_g_month = 3;
int rtc_g_day = 22;
int rtc_g_hour = 10;


# define SECRET          ""
# define UID             ""
# define CAMPUS_ID       ""
# define CLUSTER_ID      ""
# define TIME_ZONE       1  
# define WIFI_SSID       ""
# define WIFI_PASSWORD   ""


void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}



bool  ft_fetch_exams(void)
{
    const char* server PROGMEM = "api.intra.42.fr";
    String      auth_request;
    String      server_response;
    int         i;
    String      token;
    String      month;
    String      day;
    String      api_call;

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi connection lost. Reconnecting...");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(1000);
            Serial.println("Connecting to WiFi...");
        }
        Serial.println("Connected to WiFi");
    }
    WiFiClientSecure client;
    client.setInsecure();
    if (!client.connect(server, 443))
    {
        Serial.println("Connection to server failed");
        client.stop();
        return (false);
    }
    auth_request = "grant_type=client_credentials&client_id=";
    auth_request += UID;
    auth_request += "&client_secret=";
    auth_request += SECRET;
    client.print("POST https://api.intra.42.fr/oauth/token HTTP/1.1\r\n");
    client.print("Host: ");
    client.println(server);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(auth_request.length());
    client.println();
    client.println(auth_request);
    auth_request.clear();
    server_response = client.readStringUntil('}');
    if (server_response.length() <= 0)
    {
        Serial.println("Error! Server responce to the Access Token request was not received\n");
        client.stop();
        return (false);
    }
    Serial.println(server_response);
    i = server_response.indexOf("{\"access_token\":\"");
    if (i == -1)
    {
        Serial.println("Error! Server responce came without the Access Token\n");
        client.stop();
        return (false);
    }
    token = server_response.substring(i + 17, i + 81);
    Serial.println("\n");
    Serial.println(token);
    Serial.println("\n");
    server_response.clear();

    if (rtc_g_month < 10)
        month = "0" + String(rtc_g_month);
    else
        month = String(rtc_g_month);
    if (rtc_g_day < 10)
        day = "0" + String(rtc_g_day);
    else
        day = String(rtc_g_day);

    api_call = "https://api.intra.42.fr/v2/campus/";
    api_call += CAMPUS_ID;
    api_call += "/exams?filter[location]=";
    api_call += CLUSTER_ID;
    api_call += "&range[begin_at]=";
    api_call += String(rtc_g_year) + "-" + month + "-" + day + "T05:00:00.000Z,";
    api_call += String(rtc_g_year) + "-" + month + "-" + day + "T21:00:00.000Z";
    
    client.print("GET ");
    client.print(api_call);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.print("Authorization: Bearer ");
    client.println(token);
    client.println("Connection: close");
    client.println();
    
    server_response = client.readStringUntil(']');                                                        // забираем в память целиком всё сообщение сервера
    Serial.println(server_response);                                                                      // для наглядности целиком выводим сообщение сервера в Серийный монитор
    Serial.println("\n");
    if (server_response.length() <= 900)                                                                  // если сообщение пустое, то произошла ошибка
    {
        Serial.println("Error! Server responce to the Exam Time request was not received\n");
        client.stop();
        return (false);
    }
    if (server_response.indexOf("\"begin_at\":\"") == -1)                                                 // если в сообщении нет упоминания о начале экзамена, то и экзамена сегодня нет
    {
        Serial.println("As of now, no exams are planned for today\n");
//        rtc_g.exam_state = false;
        client.stop();
        return (true);
    }
    else                                                                                                  // упоминание экзамена нашлось - значит экзамен сегодня будет или уже был
    { 
        i = 0;
        while (i != -1)                                                                                   // check the server response until there are no instances of "begin_at" (-1 means "Not found")
        {
            i = server_response.indexOf("\"begin_at\":\"");                                               // find the 1st instance of "begin_at" in the the server response string
            int rtc_g_exam_start_hour = server_response.substring(i + 23, i + 25).toInt() + TIME_ZONE;
            int rtc_g_exam_start_minutes = server_response.substring(i + 26, i + 28).toInt();
            Serial.printf("Exam START time is %d:%d", rtc_g_exam_start_hour, rtc_g_exam_start_minutes);
            Serial.println();
            i = server_response.indexOf("\"end_at\":\"");
            int rtc_g_exam_end_hour = server_response.substring(i + 21, i + 23).toInt() + TIME_ZONE;
            int rtc_g_exam_end_minutes = server_response.substring(i + 24, i + 26).toInt();
            Serial.printf("Exam END time is %d:%d", rtc_g_exam_end_hour, rtc_g_exam_end_minutes);
            Serial.println("\n");
            if (rtc_g_exam_start_hour < rtc_g_hour)                                                       // checking if this information is about past exam or future exam today
                server_response = server_response.substring(i + 34);                                      // if about the past exam then delete this information from the string and check next instance of "begin_at" 
            else
            {
                Serial.println("Active Exam found!\n");
//                rtc_g.exam_state = true;
                client.stop();
                return (true);
            }
            if (server_response.indexOf("\"begin_at\":\"") == -1)                                         // if, after all the deletions, there are no more "begin_at", it means that all the discovered exams have already passed
                break;
        }
        Serial.println("All the exams planned for today have already passed. No more exams today.\n");
//        rtc_g.exam_state = false;                                                                       // all the instances of "begin_at" in the server response were about past exams, so no more exams today
    }
    client.stop();
    delay(120000);
    return (true);
}



void loop() {
    ft_fetch_exams();
}
 
