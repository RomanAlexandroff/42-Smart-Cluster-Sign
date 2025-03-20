/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   intra_interaction.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/09 13:01:00 by raleksan          #+#    #+#             */
/*   Updated: 2024/12/09 17:20:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42-Smart-Cluster-Sign.h"

/*
*   Does not check the exam subscribers unless it is
*   less than SUBS_CHECK_LIMIT_MS before the exam.
*/
static void  check_exam_subscribers(String &server_response)
{
    int i;
    int subscribers;

    i = 0;
    subscribers = 0;
    if (time_till_event(rtc_g.exam_start_hour, rtc_g.exam_start_minutes) > SUBS_CHECK_LIMIT_MS)
        return;
    watchdog_reset();
    i = server_response.indexOf("\"nbr_subscribers\":\"");
    if (i == NOT_FOUND)
        return;
    subscribers = server_response.substring(i + 18, i + 19).toInt();
    if (subscribers == 0)
    {
        DEBUG_PRINTF("\n[INTRA] Noone has subscribed to this exam. Exam mode canceled.\n\n");
        rtc_g.exam_status = false;
    }
    else
        DEBUG_PRINTF("\n[INTRA] %d subscribers detected. Continuing with the Exam mode.\n\n", subscribers);
}

static void  get_exam_time(String &server_response)
{
    int i;

    i = 0;
    watchdog_reset();
    while (i != NOT_FOUND)
    {
        i = server_response.indexOf("\"begin_at\":\"");
        rtc_g.exam_start_hour = server_response.substring(i + 23, i + 25).toInt() + TIME_ZONE;
        rtc_g.exam_start_minutes = server_response.substring(i + 26, i + 28).toInt();
        i = server_response.indexOf("\"end_at\":\"");
        com_g.exam_end_hour = server_response.substring(i + 21, i + 23).toInt() + TIME_ZONE;
        com_g.exam_end_minutes = server_response.substring(i + 24, i + 26).toInt();
        DEBUG_PRINTF("\n[INTRA] EXAM STATUS: Exam information detected\n");
        DEBUG_PRINTF("-- Begins at %d:", rtc_g.exam_start_hour);
        DEBUG_PRINTF("%d0\n", rtc_g.exam_start_minutes);
        DEBUG_PRINTF("-- Ends at %d:", com_g.exam_end_hour);
        DEBUG_PRINTF("%d0\n", com_g.exam_end_minutes);
        if (com_g.exam_end_hour <= com_g.hour)
            server_response = server_response.substring(i + 87);              // remove data of a past exam
        else
        {
            DEBUG_PRINTF("\n[INTRA] EXAM STATUS: Active Exam found!\n\n");
            rtc_g.exam_status = true;
            return;
        }
        if (server_response.indexOf("\"begin_at\":\"") == NOT_FOUND)
            break;
    }
    DEBUG_PRINTF("\n[INTRA] EXAM STATUS: All the detected exams have already passed.\n\n");
    rtc_g.exam_status = false;
}

static bool  handle_exams_info(void)
{
    int     i;
    String  server_response;

    watchdog_reset();
    i = WD_TIMEOUT_MS - SERVER_WAIT_MS;
    if (i < 1000)
        i = 1000;
    while (Intra_client.available() && i)
    {
        server_response += Intra_client.readString();
        i--;
    }
    #ifdef EXAM_SIMULATION
        server_response += exam_simulation();
    #endif
    DEBUG_PRINTF("\n============================== SERVER RESPONSE START ==============================\n\n");
    DEBUG_PRINTF("%s", server_response.c_str());
    DEBUG_PRINTF("=============================== SERVER RESPONSE END ===============================\n\n");
    if (server_response.length() <= 0)
    {
        DEBUG_PRINTF("\n[INTRA] Error! Server response to the Exam Time request was not received\n\n");
        return (false);
    }
    if (server_response.indexOf("\"begin_at\":\"") == NOT_FOUND)
    {
        DEBUG_PRINTF("\n[INTRA] EXAM STATUS: As of now, there are no upcoming exams today\n\n");
        rtc_g.exam_status = false;
        return (true);
    }
    else
    {
        get_exam_time(server_response);
        check_exam_subscribers(server_response);
    }
    return (true);
}

static void  request_exams_info(const char* server, String* token)
{
    String  day;
    String  month;
    String  api_call;

    watchdog_reset();
    if (com_g.month < 10)
        month = "0" + String(com_g.month);
    else
        month = String(com_g.month);
    if (com_g.day < 10)
        day = "0" + String(com_g.day);
    else
        day = String(com_g.day);
    api_call = "https://api.intra.42.fr/v2/campus/";
    api_call += CAMPUS_ID;
    api_call += "/exams?filter[location]=";
    api_call += CLUSTER_ID;
    api_call += "&range[begin_at]=";
    api_call += String(com_g.year) + "-" + month + "-" + day + "T05:00:00.000Z,";
    api_call += String(com_g.year) + "-" + month + "-" + day + "T21:00:00.000Z";
    Intra_client.print("GET ");
    Intra_client.print(api_call);
    Intra_client.println(" HTTP/1.1");
    Intra_client.print("Host: ");
    Intra_client.println(server);
    Intra_client.print("Authorization: Bearer ");
    Intra_client.println(*token);
    Intra_client.println("Connection: close");
    Intra_client.println();
    delay(SERVER_WAIT_MS);
}

static void get_secret_expiration(String server_response)
{
    int      i;
    uint8_t  expire_day;
    uint8_t  expire_month;
    uint16_t expire_year;

    watchdog_reset();
    i = server_response.indexOf("\"secret_valid_until\":");
    if (i == NOT_FOUND)
        DEBUG_PRINTF("[INTRA] Secret expiration date was not found in the server response\n\n");
    else
    {
        rtc_g.secret_expiration = server_response.substring(i + 21, i + 31).toInt();
        DEBUG_PRINTF("[INTRA] The secret expires on %lld (UNIX timestamp format)\n", rtc_g.secret_expiration);
        if (unix_timestamp_decoder(&expire_day, &expire_month, &expire_year))
        {
            DEBUG_PRINTF("[INTRA] The secret expires on %d.", expire_day);
            DEBUG_PRINTF("%d.", expire_month);
            DEBUG_PRINTF("%d\n", expire_year);
            DEBUG_PRINTF("[INTRA] The secret days left: %d\n\n", expiration_counter());
        }
    }
}

static String get_token(String server_response)
{
    int     i;
    String  token;

    watchdog_reset();
    i = server_response.indexOf("{\"access_token\":\"");
    if (i == NOT_FOUND)
    {
        DEBUG_PRINTF("\n[INTRA] Error! Server response came without the Access Token\n\n");
        return ("NOT_FOUND");
    }
    token = server_response.substring(i + 17, i + 81);
    DEBUG_PRINTF("\n[INTRA] Access Token has been extracted:\n%s\n", token.c_str());
    return (token);
}

static bool  handle_server_response(const char* server, String* token)
{
    int     i;
    String  server_response;

    watchdog_reset();
    i = WD_TIMEOUT_MS - SERVER_WAIT_MS;
    if (i < 1000)
        i = 1000;
    while (Intra_client.available() && i)
    {
        server_response += Intra_client.readString();
        i--;
    }
    DEBUG_PRINTF("\n============================== SERVER RESPONSE START ==============================\n\n");
    DEBUG_PRINTF("%s", server_response.c_str());
    DEBUG_PRINTF("=============================== SERVER RESPONSE END ===============================\n\n");
    if (server_response.length() <= 0)
    {
        DEBUG_PRINTF("\n[INTRA] Error! Server response to the Access Token request was not received\n\n");
        return (false);
    }
    *token = get_token(server_response);
    if (*token == "NOT_FOUND")
        return (false);
    get_secret_expiration(server_response);
    return (true);
}

static void  access_server(const char* server)
{
    String  auth_request;

    watchdog_reset();
    auth_request = "grant_type=client_credentials&client_id=";
    auth_request += UID;
    auth_request += "&client_secret=";
    auth_request += String(rtc_g.Secret);
    Intra_client.print("POST https://api.intra.42.fr/oauth/token HTTP/1.1\r\n");
    Intra_client.print("Host: ");
    Intra_client.println(server);
    Intra_client.println("Content-Type: application/x-www-form-urlencoded");
    Intra_client.print("Content-Length: ");
    Intra_client.println(auth_request.length());
    Intra_client.println();
    Intra_client.println(auth_request);
    delay(SERVER_WAIT_MS);
    auth_request.clear();
}

static bool  intra_connect(const char* server)
{
    if (WiFi.status() != WL_CONNECTED)
        wifi_connect();
    if (WiFi.status() != WL_CONNECTED)
    {
        DEBUG_PRINTF("\n[INTRA] Unable to connect to Wi-Fi\n\n");
        return (false);
    }
    watchdog_reset();
    Intra_client.setInsecure();
    Intra_client.setTimeout(20);
    if (!Intra_client.connect(server, 443))
    {
        DEBUG_PRINTF("\n[INTRA] Connection to the server failed\n\n");
        return (false);
    }
    return (true);
}

ERROR_t fetch_exams(void)
{
    const char* server PROGMEM = "api.intra.42.fr";
    String      token;

    watchdog_reset();
    if (!intra_connect(server))
        return INTRA_NO_SERVER;
    access_server(server);
    if (!handle_server_response(server, &token))
    {
        Intra_client.stop();
        return INTRA_NO_TOKEN;
    }
    request_exams_info(server, &token);
    if (!handle_exams_info())
    {
        Intra_client.stop();
        return INTRA_NO_INFO;
    }
    Intra_client.stop();
    return INTRA_OK;
}
 
