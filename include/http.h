#ifndef HTTP_H
#define HTTP_H

// HTTP Methods
#define OPTIONS "OPTIONS"
#define GET "GET"
#define HEAD "HEAD"
#define POST "POST"
#define PUT "PUT"
#define DELETE "DELETE"
#define TRACE "TRACE"
#define CONNECT "CONNECT"

// HTTP Status Codes
#define CONTINUE 100
#define SWITCHING_PROTOCOLS 101

#define OK 200
#define CREATED 201
#define ACCEPTED 202
#define NON_AUTHORITATIVE_INFORMATION 203
#define NO_CONTENT 204
#define RESET_CONTENT 205
#define PARTIAL_CONTENT 206

#define MULTIPLE_CHOICES 300
#define MOVED_PERMANENTLY 301
#define FOUND 302
#define SEE_OTHER 303
#define NOT_MODIFIED 304
#define USE_PROXY 305
#define UNUSED_306 306
#define TEMPORARY_REDIRECT 307

#define BAD_REQUEST 400
#define UNAUTHORIZED 401
#define PAYMENT_REQUIRED 402
#define FORBIDDEN 403
#define NOT_FOUND 404
#define METHOD_NOT_ALLOWED 405
#define NOT_ACCEPTABLE 406
#define PROXY_AUTHENTICATION_REQUIRED 407
#define REQUEST_TIMEOUT 408
#define CONFLICT 409
#define GONE 410
#define LENGTH_REQUIRED 411
#define PRECONDITION_FAILED 412
#define REQUEST_ENTITY_TOO_LARGE 413
#define REQUEST_URI_TOO_LONG 414
#define UNSUPPORTED_MEDIA_TYPE 415
#define REQUEST_RANGE_NOT_SATISFIABLE 416
#define EXPECTATION_FAILED 417

#define INTERNAL_SERVER_ERROR 500
#define NOT_IMPLEMENTED 501
#define BAD_GATEWAY 502
#define SERVICE_UNAVAILABLE 503
#define GATEWAY_TIMEOUT 504
#define HTTP_VERSION_NOT_SUPPORTED 505

// HTTP Headers
#define CONNECTION "Connection"
#define CONNECTION_ "connection"
#define CONTENT_LENGTH "Content-Length"
#define CONTENT_LENGTH_ "content-length"
#define CONTENT_TYPE "Content-Type"
#define CONTENT_TYPE_ "content-type"
#define HOST "Host"
#define HOST_ "host"
#define SERVER "Server"
#define SERVER_ "server"

#endif
