/*******************************************************************************
 * Copyright (c) 2012, 2013 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *   http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTAsync.h"
#include <time.h>
#include <curl/curl.h>
#include <json-c/json.h>

#if !defined(WIN32)
#include <unistd.h>
#else
#include <windows.h>
#endif


#define CLIENTID    "MyThermometer"
#define TOPIC       "home/bedroom/temperature"
#define QOS         1
#define TIMEOUT     10000L

volatile MQTTAsync_token deliveredtoken;
int get_temperature(char *);

int finished = 0;

void connlost(void *context, char *cause)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;

	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);

	printf("Reconnecting\n");
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
 		finished = 1;
	}
}


void onDisconnect(void* context, MQTTAsync_successData* response)
{
	printf("Successful disconnection\n");
	finished = 1;
}


void onSend(void* context, MQTTAsync_successData* response)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
	int rc;

	printf("Message with token value %d delivery confirmed\n", response->token);

	opts.onSuccess = onDisconnect;
	opts.context = client;

	if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start sendMessage, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
}


void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Connect failed, rc %d\n", response ? response->code : 0);
	finished = 1;
}


void onConnect(void* context, MQTTAsync_successData* response)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	int rc;

	printf("Successful connection\n");

	opts.onSuccess = onSend;
	opts.context = client;

	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	deliveredtoken = 0;

    int temp = get_temperature("Madrid");
    char payload[5];
    sprintf(payload, "%d", temp);
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(payload);
    printf("Temperature: %s\n", payload);
    fflush(stdout);
    if ((rc = MQTTAsync_sendMessage(client, TOPIC, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start sendMessage, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

}

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

int get_temperature(char *city){

    char *weather_api_key = "0dfa267b1e0ed74b8768d7d4f539f435";

    CURL *curl;
    CURLcode res;

    char url[250];
    strcpy(url, "http://api.openweathermap.org/data/2.5/weather?units=metric&q=");
    strcat(url, city);
    strcat(url, "&appid=");
    strcat(url, weather_api_key);

    curl = curl_easy_init();
    if(curl) {
        struct string s;
        init_string(&s);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        res = curl_easy_perform(curl);

        struct json_object *full = json_tokener_parse(s.ptr);
        struct json_object *main;
        struct json_object *temp;

        main = json_object_object_get(full, "main");
        temp = json_object_object_get(main, "temp");
        int ret = json_object_get_double(temp);
        free(s.ptr);

        /* always cleanup */
        curl_easy_cleanup(curl);

        return ret;
    }
    return 0;
}


int main(int argc, char* argv[])
{
	MQTTAsync client;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	MQTTAsync_token token;
	int rc;



	while(1){
	    finished = 0;
	    char *address = getenv("MOSQUITTO_URL");
	    char *username = getenv("MOSQUITTO_USERNAME");
	    char *password = getenv("MOSQUITTO_PASS");
	    char *city = getenv("TEMP_CITY");

	    if(address == NULL || username == NULL || password == NULL){
	        printf("Error, not MOSQUITTO_URL, MOSQUITTO_USERNAME or MOSQUITTO_PASS env vars not declared\n");
	        return -2;
	    }

	    if(city == NULL){
	        printf("Error, specify a TEMP_CITY env var\n");
	        return -2;
	    }

        printf("Temperature: %d\n", get_temperature(city));
        return 0;


        MQTTAsync_create(&client, address, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
        MQTTAsync_setCallbacks(client, NULL, connlost, NULL, NULL);

        conn_opts.keepAliveInterval = 20;
        conn_opts.cleansession = 1;
        conn_opts.onSuccess = onConnect;
        conn_opts.onFailure = onConnectFailure;
        conn_opts.context = client;
        conn_opts.username = username;
        conn_opts.password = password;

        if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
        {
            printf("Failed to start connect, return code %d\n", rc);
            exit(EXIT_FAILURE);
        }

        printf("Waiting for publication on topic %s for client with ClientID: %s\n", TOPIC, CLIENTID);
        while (!finished)
            usleep(1200000000L);

	}
	MQTTAsync_destroy(&client);
 	return rc;
}
