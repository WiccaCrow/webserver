#pragma once

#include <Utils.hpp>
#include <unistd.h>
#include <dirent.h>
#include <StatusCodes.hpp>
#include <Request.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <cstdio>

namespace HTTP {
// # define SIZE_FOR_CHUNKED 4096
class Response {
    std::string     _res;
    std::string     _resLeftToSend;    
    bool            _responseFormed;

    static const std::map<int, const char *>            _ErrorCode;
    static const std::map<std::string, std::string>     _ContType;

    public:
    Response();
    ~Response() {}

    bool isFormed() const {
        return _responseFormed;
    }

    void setFormed(bool formed) {
        _responseFormed = formed;
    }

    static std::map<int, const char *>          initErrorCode();
    static std::map<std::string, std::string>   initContType();

    // void resetResponse() {
    //     _res = "";
    // }

    void        HEADmethod(Request &req);
    void        GETmethod(Request &req);
    void        POSTmethod(Request &req);
    void        DELETEmethod(Request &req);

    // Вспомогательные функции для подготовки ответа внутри методов
    // Helper functions for preparing a response inside methods.
    std::string     contentForGetHead(Request &req);
    std::string     resoursePathTaker(Request &req);
    std::string     doCGI(Request &req);
    std::string     fileToResponse(std::string &resourcePath);
    std::string     listToResponse(std::string &resourcePath, Request &req);

    // Вспомогательные функции для отправления ответа из класса Client
    // Helper functions for sending a response from the Client class
    void            SetLeftToSend(size_t n);

    const char *    getErr(int nbErr);
    std::string     getContentType(std::string resourcePath);
    size_t          getResSize(void);
    const char *    getResponse(void);
    const char *    getLeftToSend(void);
    size_t          getLeftToSendSize(void);
    
    // std::string        TransferEncodingChunked(std::string buffer, size_t bufSize);
};

};

    // const char *getData(void) {
    //     std::string body = "<html><body> Hello from the other side! </body></html>";
    //     _res = "HTTP/1.1 200 OK\r\n";
    //     _res += "Content-type: text/html; charset=utf-8\r\n";
    //     _res += "Connection: keep-alive\r\n";
    //     _res += "Keep-Alive: timeout=55, max=1000\r\n";
    //     _res += "Content-length: " + to_string(body.length()) + "\r\n\r\n";
    //     _res += body;

    //     return _res.c_str();
    // }


    // Errors: уже добавлены
    // 400 403 404 405 408 411 413 414 415
    // 500 501 502 504 505
    // const char *    ErrorCli406(void);
    // const char *    ErrorCli409(void);
    // const char *    ErrorCli410(void);
    // const char *    ErrorCli412(void); // ??
    // const char *    ErrorServ503(void);


    // Описание:
    //      Формирует в переменой _res заголовки ответа.
    // Возвращаемое значение:
    //      Возвращает строку, содержащую тело ответа.
    // std::string contentForGetHead(Request &req);
    //
    //
    //
    // Описание:
    //      Создает путь до запрощенного ресурса в соответствии с файлом конфигурации.
    // Возвращаемое значение:
    //      Строка с созданным путем.
    // std::string resoursePathTaker(Request &req);
    //
    //
    //
    // Описание:
    //      В соответствии с конфигурационным файлом проверяет, задан ли путь до  CGI.
    //      Если да, то запускается функция для CGI и формируется заголовок в _res 
    //      "content length: ".
    // Возвращаемое значение:
    //      Если функция для CGI была запущена, возвращается строка, сформированная
    //      этой функцией.
    //      Иначе возвращается пустая строка.
    // std::string doCGI(Request &req);
    //
    //
    //
    // Описание:
    //      Функция открывает файл запрошенного ресурса и формирует строку из 
    //      считанных из файла данных. Функция добавляет заголовок "content-length: " в 
    //      переменную _res.
    //      В случае отсутствия ресурса _res по умолчанию формирует 404 ошибку, либо 
    //      происходит перенаправление на новый ресурс в соответствии с файлом конфигурации.
    // Возвращаемое значение:
    //      В случае успеха открытия файла возвращается строка с прочитанным содержимым 
    //      запрошенного ресурса.
    //      В случае отсутствия ресурса и не заданном перенаправлении на новый ресурс 
    //      в файле конфигурации возвращается пустая строка.
    // std::string fileToResponse(std::string &resourcePath);
    //
    //
    //
    // Описание:
    //      Функция формирует тело для ответа в HTML формате, содержащее список 
    //      содержимого Директории (с путем resourcePath).
    //      В случае успеха добавляется заголовок "content-length: " в 
    //      переменную _res.
    //      В случае возникновения ошибки при открытии Директории в переменной _res 
    //      записывается ответ с ошибкой 500.
    // Возвращаемое значение:
    //      В случае успеха возвращается строка содержащая тело для ответа.
    //      В случае возникновения ошибки при открытии Директори возвращается пустая строка.
    // std::string listToResponse(std::string &resourcePath, Request &req);
