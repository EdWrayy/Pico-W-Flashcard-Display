/* Pico HTTPS request example *************************************************
 *                                                                            *
 *  An HTTPS client example for the Raspberry Pi Pico W                       *
 *                                                                            *
 *  A simple yet complete example C application which sends a single request  *
 *  to a web server over HTTPS and reads the resulting response.              *
 *                                                                            *
 ******************************************************************************/

 #ifndef PICOHTTPS_H
 #define PICOHTTPS_H
 
 
 
 /* Options ********************************************************************/
 
 // Wireless region
 //
 //  `country` argument to cyw43_arch_init_with_country().
 //
 //  For best performance, set to local region.
 //
 //  https://www.raspberrypi.com/documentation/pico-sdk/networking.html#CYW43_COUNTRY_
 //
 #define PICOHTTPS_INIT_CYW43_COUNTRY                CYW43_COUNTRY_UK
 
 // Wireless network SSID
 #define PICOHTTPS_WIFI_SSID                         "YOUR_WIFI_SSID"
 
 // Wireless network connection timeout
 //
 //  `timeout` argument to cyw43_arch_wifi_connect_timeout_ms().
 //
 //  https://www.raspberrypi.com/documentation/pico-sdk/networking.html
 //
 #define PICOHTTPS_WIFI_TIMEOUT                      20000           // ms
 
 // Wireless network password
 //
 //  N.b. _Strongly_ recommend setting this from the environment rather than
 //  here. Environment values will have greater precedence. See CMakeLists.txt.
 //
 #ifndef PICOHTTPS_WIFI_PASSWORD
 #define PICOHTTPS_WIFI_PASSWORD                     "YOUR_WIFI_PASSWORD"
 #endif // PICOHTTPS_WIFI_PASSWORD
 
 // HTTP server hostname
 #define PICOHTTPS_HOSTNAME                          "username.github.io"
 
 // DNS response polling interval
 //
 //  Interval with which to poll for responses to DNS queries.
 //
 #define PICOHTTPS_RESOLVE_POLL_INTERVAL             100             // ms
 
 // Certificate authority root certificate
 //
 //  CA certificate used to sign the HTTP server's certificate. DER or PEM
 //  formats (char array representation).
 //
 //  This is most readily obtained via inspection of the server's certificate
 //  chain, e.g. in a browser.
 //
 #define PICOHTTPS_CA_ROOT_CERT                          \
 {                                                       \
     0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,     \
     0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f      \
 }
 //
 //  or
 //
 #define PICOHTTPS_CA_ROOT_CERT    \                                   
"-----BEGIN CERTIFICATE-----\n" \
"MIIF3jCCA8agAwIBAgIQAf1tMPyjylGoG7xkDjUDLTANBgkqhkiG9w0BAQwFADCB\n" \
"iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n" \
"cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n" \
"BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAw\n" \
"MjAxMDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNV\n" \
"BAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVU\n" \
"aGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBSU0EgQ2Vy\n" \
"dGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIK\n" \
"AoICAQCAEmUXNg7D2wiz0KxXDXbtzSfTTK1Qg2HiqiBNCS1kCdzOiZ/MPans9s/B\n" \
"3PHTsdZ7NygRK0faOca8Ohm0X6a9fZ2jY0K2dvKpOyuR+OJv0OwWIJAJPuLodMkY\n" \
"tJHUYmTbf6MG8YgYapAiPLz+E/CHFHv25B+O1ORRxhFnRghRy4YUVD+8M/5+bJz/\n" \
"Fp0YvVGONaanZshyZ9shZrHUm3gDwFA66Mzw3LyeTP6vBZY1H1dat//O+T23LLb2\n" \
"VN3I5xI6Ta5MirdcmrS3ID3KfyI0rn47aGYBROcBTkZTmzNg95S+UzeQc0PzMsNT\n" \
"79uq/nROacdrjGCT3sTHDN/hMq7MkztReJVni+49Vv4M0GkPGw/zJSZrM233bkf6\n" \
"c0Plfg6lZrEpfDKEY1WJxA3Bk1QwGROs0303p+tdOmw1XNtB1xLaqUkL39iAigmT\n" \
"Yo61Zs8liM2EuLE/pDkP2QKe6xJMlXzzawWpXhaDzLhn4ugTncxbgtNMs+1b/97l\n" \
"c6wjOy0AvzVVdAlJ2ElYGn+SNuZRkg7zJn0cTRe8yexDJtC/QV9AqURE9JnnV4ee\n" \
"UB9XVKg+/XRjL7FQZQnmWEIuQxpMtPAlR1n6BB6T1CZGSlCBst6+eLf8ZxXhyVeE\n" \
"Hg9j1uliutZfVS7qXMYoCAQlObgOK6nyTJccBz8NUvXt7y+CDwIDAQABo0IwQDAd\n" \
"BgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/BAQDAgEGMA8G\n" \
"A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAFzUfA3P9wF9QZllDHPF\n" \
"Up/L+M+ZBn8b2kMVn54CVVeWFPFSPCeHlCjtHzoBN6J2/FNQwISbxmtOuowhT6KO\n" \
"VWKR82kV2LyI48SqC/3vqOlLVSoGIG1VeCkZ7l8wXEskEVX/JJpuXior7gtNn3/3\n" \
"ATiUFJVDBwn7YKnuHKsSjKCaXqeYalltiz8I+8jRRa8YFWSQEg9zKC7F4iRO/Fjs\n" \
"8PRF/iKz6y+O0tlFYQXBl2+odnKPi4w2r78NBc5xjeambx9spnFixdjQg3IM8WcR\n" \
"iQycE0xyNN+81XHfqnHd4blsjDwSXWXavVcStkNr/+XeTWYRUc+ZruwXtuhxkYze\n" \
"Sf7dNXGiFSeUHM9h4ya7b6NnJSFd5t0dCy5oGzuCr+yDZ4XUmFF0sbmZgIn/f3gZ\n" \
"XHlKYC6SQK5MNyosycdiyA5d9zZbyuAlJQG03RoHnHcAP9Dc1ew91Pq7P8yF1m9/\n" \
"qS3fuQL39ZeatTXaw2ewh0qpKJ4jjv9cJ2vhsE/zB+4ALtRZh8tSQZXq9EfX7mRB\n" \
"VXyNWQKV3WKdwrnuWih0hKWbt5DHDAff9Yk2dDLWKMGwsAvgnEzDHNb842m1R0aB\n" \
"L6KCq9NjRHDEjf8tM7qtj3u1cIiuPhnPQCjY/MiQu12ZIvVS5ljFH4gxQ+6IHdfG\n" \
"jjxDah2nGN59PRbxYvnKkKj9\n" \
"-----END CERTIFICATE-----\n"

 #define PICOHTTPS_ALTCP_CONNECT_POLL_INTERVAL       100             // ms
 
 // TCP + TLS idle connection polling interval
 //
 //  Interval with which to poll application (i.e. call registered polling
 //  callback function) when TCP + TLS connection is idle.
 //
 //  The callback function should be registered with altcp_poll(). The polling
 //  interval is given in units of 'coarse grain timer shots'; one shot
 //  corresponds to approximately 500 ms.
 //
 //  https://www.nongnu.org/lwip/2_1_x/group__altcp.html
 //
 #define PICOHTTPS_ALTCP_IDLE_POLL_INTERVAL          2               // shots
 
 // HTTP request
 //
 //  Plain-text HTTP request to send to server
 //
 #define PICOHTTPS_REQUEST\
    "GET /anki-csv-decks/cards.csv HTTP/1.1\r\n"\
    "Host: " PICOHTTPS_HOSTNAME "\r\n"\
    "Connection: close\r\n"\
    "\r\n"
 
 
 // HTTP response polling interval
 //
 //  Interval with which to poll for HTTP response from server.
 //
 #define PICOHTTPS_HTTP_RESPONSE_POLL_INTERVAL       100             // ms
 
 // Mbed TLS debug levels
 //
 //  Seemingly not defined in Mbed TLS‽
 //
 //  https://github.com/Mbed-TLS/mbedtls/blob/62e79dc913325a18b46aaea554a2836a4e6fc94b/include/mbedtls/debug.h#L141
 //
 #define PICOHTTPS_MBEDTLS_DEBUG_LEVEL               3
 
 
 /* Macros *********************************************************************/
 
 // Array length
 #define LEN(array) (sizeof array)/(sizeof array[0])
 
 
 /* My additions ***************************************************************/
#define MAX_RESPONSE_SIZE 16384
extern char response_buffer[MAX_RESPONSE_SIZE];
extern size_t response_offset;
extern bool response_complete;
 
 
 
 /* Data structures ************************************************************/
 
 // lwIP errors
 //
 //  typedef here to make source of error code more explicit
 //
 typedef err_t lwip_err_t;
 
 // Mbed TLS errors
 //
 //  typedef here to make source of error code more explicit
 //
 typedef int mbedtls_err_t;
 
 // TCP connection callback argument
 //
 //  All callbacks associated with lwIP TCP (+ TLS) connections can be passed a
 //  common argument. This is intended to allow application state to be accessed
 //  from within the callback context. The argument should be registered with
 //  altcp_arg().
 //
 //  The following structure is used for this argument in order to supply all
 //  the relevant application state required by the various callbacks.
 //
 //  https://www.nongnu.org/lwip/2_1_x/group__altcp.html
 //
 struct altcp_callback_arg{
 
     // TCP + TLS connection configurtaion
     //
     //  Memory allocated to the connection configuration structure needs to be
     //  freed (with altcp_tls_free_config) in the connection error callback
     //  (callback_altcp_err).
     //
     //  https://www.nongnu.org/lwip/2_1_x/group__altcp.html
     //  https://www.nongnu.org/lwip/2_1_x/group__altcp__tls.html
     //
     struct altcp_tls_config* config;
 
     // TCP + TLS connection state
     //
     //  Successful establishment of a connection needs to be signaled to the
     //  application from the connection connect callback
     //  (callback_altcp_connect).
     //
     //  https://www.nongnu.org/lwip/2_1_x/group__altcp.html
     //
     bool connected;
 
     // Data reception acknowledgement
     //
     //  The amount of data acknowledged as received by the server needs to be
     //  communicated to the application from the connection sent callback
     //  (callback_altcp_sent) for validatation of successful transmission.
     //
     u16_t acknowledged;
 
 };

 
 
 
 
 /* Functions ******************************************************************/
 

// Fetch CSV file from server
// @ return         `true` on success
// @ return         `false` on failure
// The main function  of the program. It initializes the standard I/O over USB, the CYW43 wireless hardware, connects to the network, resolves the server hostname.
// Establishes a TCP + TLS connection with the server, sends an HTTP request, and waits for a response. 
 bool fetch_csv(void);

 // Initialise standard I/O over USB
 //
 //  @return         `true` on success
 //
 bool init_stdio(void);
 
 // Initialise Pico W wireless hardware
 //
 //  @return         `true` on success
 //
 bool init_cyw43(void);
 
 // Connect to wireless network
 //
 //  @return         `true` on success
 //
 bool connect_to_network(void);
 
 // Resolve hostname
 //
 //  @param ipaddr   Pointer to an `ip_addr_t` where the resolved IP address
 //                  should be stored.
 //
 //  @return         `true` on success
 //
 bool resolve_hostname(ip_addr_t* ipaddr);
 
 // Free TCP + TLS protocol control block
 //
 //  Memory allocated for a protocol control block (with altcp_tls_new) needs to
 //  be freed (with altcp_close).
 //
 //  @param pcb      Pointer to a `altcp_pcb` structure to be freed
 //
 void altcp_free_pcb(struct altcp_pcb* pcb);
 
 // Free TCP + TLS connection configuration
 //
 //  Memory allocated for TCP + TLS connection configuration (with
 //  altcp_tls_create_config_client) needs to be freed (with
 //  altcp_tls_free_config).
 //
 //  @param config   Pointer to a `altcp_tls_config` structure to be freed
 //
 void altcp_free_config(struct altcp_tls_config* config);


 
 // Free TCP + TLS connection callback argument
 //
 //  The common argument passed to lwIP connection callbacks must remain in
 //  scope for the duration of all callback contexts (i.e. connection lifetime).
 //  As such, it cannot be declared with function scope when registering the
 //  callback, but rather should be allocated on the heap. This implies the
 //  allocated memory must be freed on connection close.
 //
 //  @param arg      Pointer to a `altcp_callback_arg` structure to be freed
 //
 void altcp_free_arg(struct altcp_callback_arg* arg);
 
 // Establish TCP + TLS connection with server
 //
 //  @param ipaddr   Pointer to an `ip_addr_t` containing the server's IP
 //                  address
 //  @param pcb      Double pointer to a `altcp_pcb` structure where the
 //                  protocol control block for the established connection
 //                  should be stored.
 //
 //  @return         `true` on success
 //
 bool connect_to_host(ip_addr_t* ipaddr, struct altcp_pcb** pcb);
 
 // Send HTTP request
 //
 //  @param pcb      Pointer to a `altcp_pcb` structure containing the TCP + TLS
 //                  connection PCB to the server.
 //
 //  @return         `true` on success
 //
 bool send_request(struct altcp_pcb* pcb);
 
 // DNS response callback
 //
 //  Callback function fired on DNS query response.
 //
 //  Registered with dns_gethostbyname().
 //
 //  https://www.nongnu.org/lwip/2_1_x/group__dns.html
 //
 void callback_gethostbyname(
     const char* name,
     const ip_addr_t* resolved,
     void* ipaddr
 );
 
 // TCP + TLS connection error callback
 //
 //  Callback function fired on TCP + TLS connection fatal error.
 //
 //  Registered with altcp_err().
 //
 //  https://www.nongnu.org/lwip/2_1_x/group__altcp.html
 //
 void callback_altcp_err(void* arg, lwip_err_t err);
 
 // TCP + TLS connection idle callback
 //
 //  Callback function fired on idle TCP + TLS connection.
 //
 //  Registered with altcp_err().
 //
 //  https://www.nongnu.org/lwip/2_1_x/group__altcp.html
 //
 lwip_err_t callback_altcp_poll(void* arg, struct altcp_pcb* pcb);
 
 // TCP + TLS data acknowledgement callback
 //
 //  Callback function fired on acknowledgement of data reception by server over
 //  a TCP + TLS connection.
 //
 //  Registered with altcp_sent().
 //
 //  https://www.nongnu.org/lwip/2_1_x/group__altcp.html
 //
 lwip_err_t callback_altcp_sent(void* arg, struct altcp_pcb* pcb, u16_t len);
 
 // TCP + TLS data reception callback
 //
 //  Callback function fired on reception of data from server over a TCP +
 //  TLS connection.
 //
 //  Registered with altcp_recv().
 //
 //  https://www.nongnu.org/lwip/2_1_x/group__altcp.html
 //
 lwip_err_t callback_altcp_recv(
     void* arg,
     struct altcp_pcb* pcb,
     struct pbuf* buf,
     lwip_err_t err
 );
 
 // TCP + TLS connection establishment callback
 //
 //  Callback function fired on successful establishment of TCP + TLS connection.
 //
 //  Registered with altcp_connect().
 //
 //  https://www.nongnu.org/lwip/2_1_x/group__altcp.html
 //
 lwip_err_t callback_altcp_connect(
     void* arg,
     struct altcp_pcb* pcb,
     lwip_err_t err
 );
 
 
 
 #endif //PICOHTTPS_H