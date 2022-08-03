# webserver

# Compilation
```bash
# Compile server
make

# Compile server with daemon mode
make daemon

# Compile cgi test script (gcc required)
make cgi

# Prepare test files
make test

```

# Preparation

To run from any folder add server folder to the `$PATH`

```bash
# Temporary (opened terminal session)
export PATH="path-to-serv-dir:$PATH"

# Permanent (zsh)
echo 'export PATH="path-to-serv-dir:$PATH"' > ~/.zshrc

# Permanent (bash)
echo 'export PATH="path-to-serv-dir:$PATH"' > ~/.bashrc

```

# Run

Here's some run examples:

```bash

# Run with your config 
./webserv path-to-conf.json

# Or use default (default/conf.json)
./webserv 

# Background process ($PATH modification required)
webserv --daemon

# Log levels
./webserv -l side

# Directory with logs ($PATH modification required)
webserv -d <logdir>

# Help
./webserv -h

```

---

## Directives

There's a list of all directives that server supports.

#### [servers](#::servers)
#### [settings](#::settings)
#### [listen](#serverblock::listen)
#### [server_names](#serverblock::server_names)
#### [locations](#serverblock::locations)
#### [error_pages](#location::error_pages)
#### [cgi](#location::cgi)
#### [methods_allowed](#location::methods_allowed)
#### [cgi_methods](#location::cgi_methods)
#### [post_max_body](#location::post_max_body)
#### [autoindex](#location::autoindex)
#### [index](#location::index)
#### [root](#location::root)
#### [alias](#location::alias)
#### [add_headers](#location::add_headers)
#### [auth_basic](#location::auth_basic)
#### [realm](#auth_basic::realm)
#### [user_file](#auth_basic::user_file)
#### [redirect](#location::redirect)
#### [code](#redirect::code)
#### [url](#redirect::url)
#### [max_wait_conn](#settings::max_wait_conn)
#### [max_requests](#settings::max_requests)
#### [max_client_timeout](#settings::max_client_timeout)
#### [max_gateway_timeout](#settings::max_gateway_timeout)
#### [session_lifetime](#settings::session_lifetime)
#### [max_uri_length](#settings::max_uri_length)
#### [max_header_field_length](#settings::max_header_field_length)
#### [worker_timeout](#settings::worker_timeout)
#### [workers](#settings::workers)
#### [chunk_size](#settings::chunk_size)
#### [max_reg_file_size](#settings::max_reg_file_size)
#### [max_range_size](#settings::max_range_size)
#### [max_reg_upload_size](#settings::max_reg_upload_size)
#### [blind_proxy](#settings::blind_proxy)
#### [cookie_http_only](#settings::cookie_http_only)


---

## **::servers**

```
Type: Object
Syntax: servers: { serverblock1: { ... }, ...}
Default: Required, 1 block at least
Context: global

Example: see config file

Description: Contains serverblock configurations
```

---

### **serverblock::listen**

```
Type: String
Syntax: listen: "ip:port"
Default: "0.0.0.0:8080"
Context: serverblock

Example: 
listen: "192.168.0.2:5656"

Description: Contains ip address used by serverblock
```

---

### **serverblock::server_names**

```
Type: Array
Syntax: server_names: [ "name1", "name2", ... ]
Default: None
Context: serverblock

Example: 
server_names: [ "localhost", "webserv.com" ]

Description: Contains hostnames used by serverblock to define which block should process request.
```

---

### **serverblock::locations**

```
Type: Object
Syntax: locations: { loc1: { ... }, ...}
Default: Default configuration for /
Context: serverblock

Example: see config file

Description: Contains locations configurations.
```

---

### **location::error_pages**

```
Type: Object
Syntax: error_pages: {
    code(Number): path(String), 
    ...
}
Default: None
Context: location

Example: 

error_pages: {
    404: "~/error.jpeg", 
    400: "./errors/error400.html", 
}

Description: Contains list of pages that should be returned in case of error.
```

---

### **location::cgi**

```
Type: Object
Syntax: cgi: {
    extension(String): path_to_exec(String), 
    ...
}
Default: None
Context: location

Example: 

"cgi": {
    ".pl": "/usr/bin/perl",
    ".cgi": ""
}

Description: Contains map of script extensions and executables script should be passed to.

For compiled scripts extension should be changed to .cgi
and executable field must be empty
```

---

### **location::methods_allowed**

```
Type: Array
Syntax: methods_allowed: [
    "Method1", "Method2",  
    ...
]
Default: [ "GET", "HEAD", "OPTIONS", "POST" ]
Context: location

Example: 

methods_allowed: [
    "GET", "PUT"
]

Description: Contains list of the allowed methods of the current location
```

---

### **location::cgi_methods**

```
Type: Array
Syntax: cgi_methods: [
    "Method1", "Method2",  
    ...
]
Default: [ "GET", "HEAD", "POST" ]
Context: location

Example: 

cgi_methods: [
    "GET", "PUT"
]

Description: Contains list of the allowed methods of cgi in current location
```

---

### **location::post_max_body**

```
Type: String
Syntax: post_max_body: "size"
Default: "200 B"
Context: location

Examples: 

post_max_body: "100 B"
post_max_body: "20 MiB"
post_max_body: "1GB"

Description: Contains max size of body client could upload to the server
```

---

### **location::autoindex**

```
Type: Boolean
Syntax: autoindex: true | false
Default: false
Context: location

Examples: autoindex: true

Description: Enables\Disables directory listing in current location
```

---

### **location::index**

```
Type: Array
Syntax: index: [ "file1", "file2" ]
Default: None
Context: location

Examples: index: [ "index.html", "pic.jpeg" ]

Description: Contains list of index files of the current directory
```

---

### **location::root**

```
Type: String
Syntax: root: "path-to-dir"
Default: Required (or alias should present)
Context: location

Examples: root: "/Users/user/"

Description: Contains path to the served directory

Explanation: if location's path is /about and it has root
/example then request /about/index.html would be processed as /example/about/index.html
```

`If location name ends with / then it'll be added to the path in the root directory.` <br>
`If not trailing slash will be removed from the root.`

---

### **location::alias**

```
Type: String
Syntax: alias: "path-to-dir"
Default: Required (or alias should present)
Context: location

Examples: alias: "/Users/user/"

Description: Contains path to the served directory.

Explanation: if location's path is /about and it has alias /example then request /about/index.html would be processed as /example/index.html
```

`If location name ends with / then it'll be added to the path in the alias directory.` <br>
`If not trailing slash will be removed from the alias.`

---

### **location::add_headers**

```
Type: Array
Syntax: add_headers: [ "header1:value1", ... ]
Default: None
Context: location

Examples: add_headers: [ "Set-cookie: key1=val1;" ]

Description: Contains list of additonal headers that should be returned with the response
```

---

## **location::auth_basic**

```
Type: Object
Syntax: auth_basic: { ... }
Default: None
Context: location

Examples: 

"auth_basic": {
    "realm": "eternity",
    "user_file": ".htpasswd"
}

Description: Contains authentication configuration
```

---

### **auth_basic::realm**

```
Type: String
Syntax: realm: "name"
Default: None
Context: auth_basic

Examples: 

"realm": "eternity",

Description: Defines realm name of the basic authentication
```

---

### **auth_basic::user_file**

```
Type: String
Syntax: user_file: "path-to-file"
Default: None
Context: auth_basic

Examples: 

"user_file": "./.htpasswd",

Description: Defines the path to the file with user basic authentication data. 
```

---

## **location::redirect**

```
Type: Object
Syntax: redirect: { ... }
Default: None
Context: location

Examples: 

"redirect": {
    "code": 301
    "url" : "https://google.com"
}

Description: Contains redirect configuration
```

---

### **redirect::code**

```
Type: Number
Syntax: code: "path-to-file"
Default: None
Context: redirect

Examples: 

"code": 307

Description: Defines redirect status code (3xx only). 
```

---

### **redirect::url**

```
Type: String
Syntax: url: "redirect-url"
Default: None
Context: redirect

Examples: 

"url": "https://example.org"

Description: Defines redirect url (3xx only). 
```

---



## **::settings**

```
Type: Object
Syntax: settings: { key1: val1, ... }
Default: Default values of all related directives
Context: global

Example: see config file

Description: Contains directives related to the server
```

---

### **settings::max_wait_conn**

```
Type: Number
Syntax: max_wait_conn: 100
Default: 128
Context: settings

Description: Max number of clients waiting to be accepted
```

---

### **settings::max_requests**

```
Type: Number
Syntax: max_requests: 50
Default: 100
Context: settings

Description: Max number of requests that client could send before server closes connection.
```

---

### **settings::max_client_timeout**

```
Type: Number
Syntax: max_client_timeout: 300
Default: 100
Context: settings

Description: Max client idle timeout (sec) before server closes connection.
```

---

### **settings::max_gateway_timeout**

```
Type: Number
Syntax: max_gateway_timeout: 300
Default: 100
Context: settings

Description: Max gateway (cgi\proxy) idle timeout (sec) before server closes connection.
```

---

### **settings::session_lifetime**

```
Type: Number
Syntax: session_lifetime: 10000
Default: 86400
Context: settings

Description: Defines sessions max-age (sec).
```

---

### **settings::max_uri_length**

```
Type: Number
Syntax: max_uri_length: 512
Default: 1024
Context: settings

Description: Defines max length of the uri in start line of the request.
```

---

### **settings::max_header_field_length**

```
Type: Number
Syntax: max_header_field_length: 512
Default: 2048
Context: settings

Description: Defines max length of the header's value.
```

---

### **settings::worker_timeout**

```
Type: Number
Syntax: worker_timeout: 50000
Default: 10000
Context: settings

Description: Defines idle timeout (nanosec) for thread when no 
```

`This value should be increased carefully as large value could decrease server performance`

---

### **settings::workers**

```
Type: Number
Syntax: workers: 5
Default: 3
Context: settings

Description: Defines number of threads (workers) that processing requests.
```

`This value should be increased carefully as CPU loading increases in direct ratio.
Optiomal value is number of CPU cores twice`

---

### **settings::chunk_size**

```
Type: String
Syntax: chunk_size: "size"
Default: "200 B"
Context: settings

Examples: 

chunk_size: "100 B"
chunk_size: "20 MiB"
chunk_size: "1KB"

Description: Defines chunk size server used when performing chunked body of response

```

`This value should be increased carefully as RAM loading increases in direct ratio`

---

### **settings::max_reg_file_size**

```
Type: String
Syntax: max_reg_file_size: "size"
Default: "200 B"
Context: settings

Examples: 

max_reg_file_size: "100 B"
max_reg_file_size: "20 MiB"
max_reg_file_size: "1KB"

Description: Defined threshold size server used to decide whether body should be sent by chunks
```

`This value should be increased carefully as RAM loading increases in direct ratio`

---

### **settings::max_range_size**

```
Type: String
Syntax: max_range_size: "size"
Default: "200 B"
Context: settings

Examples: 

max_range_size: "100 B"
max_range_size: "20 MiB"
max_range_size: "1KB"

Description: Defined max range size used to limit range body
```

`This value should be increased carefully as RAM loading increases in direct ratio`

---

### **settings::max_reg_upload_size**

```
Type: String
Syntax: max_reg_upload_size: "size"
Default: "200 B"
Context: settings

Examples: 

max_reg_upload_size: "100 B"
max_reg_upload_size: "5 MiB"
max_reg_upload_size: "1KB"

Description: Defined threshold used to decide whether server should read body to the temporary file.
```

`This value should be increased carefully as RAM loading increases in direct ratio`

---

### **settings::blind_proxy**

```
Type: Boolean
Syntax: blind_proxy: true | false
Default: false
Context: location

Examples: blind_proxy: true

Description: Enables\Disables most of the validations when proxying.
```

---

### **settings::cookie_http_only**

```
Type: Boolean
Syntax: cookie_http_only: true | false
Default: false
Context: location

Examples: cookie_http_only: true

Description: Enables\Disables http-only parameter of the cookies
```

---
