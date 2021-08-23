#include "WebSerial.h"

void WebSerialClass::begin(AsyncWebServer *server, const char* url){
    _server = server;
    _ws = new AsyncWebSocket("/webserialws");

    _server->on(url, HTTP_GET, [](AsyncWebServerRequest *request){
        // Send Webpage
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", WEBSERIAL_HTML, WEBSERIAL_HTML_SIZE);
        response->addHeader("Content-Encoding","gzip");
        request->send(response);        
    });

    _ws->onEvent([&](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) -> void {
        if(type == WS_EVT_CONNECT){
            #if defined(WEBSERIAL_DEBUG)
                DEBUG_WEB_SERIAL("Client connection received");
            #endif
        } else if(type == WS_EVT_DISCONNECT){
            #if defined(WEBSERIAL_DEBUG)
                DEBUG_WEB_SERIAL("Client disconnected");
            #endif
        } else if(type == WS_EVT_DATA){
            #if defined(WEBSERIAL_DEBUG)
                DEBUG_WEB_SERIAL("Received Websocket Data");
            #endif
            if(_RecvFunc != NULL){
                _RecvFunc(data, len);
            }
        }
    });

    _server->addHandler(_ws);

    Buffer = "";

    #if defined(WEBSERIAL_DEBUG)
        DEBUG_WEB_SERIAL("Attached AsyncWebServer along with Websockets");
    #endif
}

void WebSerialClass::msgCallback(RecvMsgHandler _recv){
    _RecvFunc = _recv;
}

size_t WebSerialClass::write(uint8_t character)
{
    return this->write(&character, 1);
}

size_t WebSerialClass::write(const char *str)
{
    return this->write((const uint8_t *) str, strlen(str));
}

size_t WebSerialClass::write(const uint8_t *buffer, size_t size)
{
    for (uint16_t Contador = 0; Contador < size; Contador++)
    {    
        Buffer += (char)buffer[Contador];
    }

    return size;
}

size_t WebSerialClass::Try_Send_Buffer()
{
    size_t Retorno = 0;
    if (_ws->availableForWriteAll() == true)
    {
        AsyncWebSocket::AsyncWebSocketClientLinkedList Clientes = _ws->getClients();
        size_t Maior_Fila_Cliente = 0;
        
        for (const auto &c : Clientes)
        {
            if (c->queueLength() > Maior_Fila_Cliente)
            {
                Maior_Fila_Cliente = c->queueLength();
            }
        }

        if (Maior_Fila_Cliente <= 1)
        {
            _ws->textAll(Buffer);
            Retorno = Buffer.length();
            Buffer = "";
        }
    }
    return Retorno;
}

void WebSerialClass::Loop()
{
    Try_Send_Buffer();
}

#if defined(WEBSERIAL_DEBUG)
    void WebSerialClass::DEBUG_WEB_SERIAL(const char* message){
        Serial.println("[WebSerial] "+message);
    }
#endif

WebSerialClass WebSerial;