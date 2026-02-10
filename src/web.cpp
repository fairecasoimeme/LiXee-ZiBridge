#include <Arduino.h>
#include <ArduinoJson.h>

#ifndef ESP32S3
  #include <ETH.h>
#endif
#include "WiFi.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include "LittleFS.h"
#include "config.h"
#include "log.h"

#include <esp_task_wdt.h>
#include <Update.h>

#ifndef ESP32S3
  #include <esp_int_wdt.h>
  #include "esp_adc_cal.h"
#endif

extern struct ConfigSettingsStruct ConfigSettings;
extern unsigned long timeLog;

AsyncWebServer serverWeb(80);

volatile bool needReboot = false;

void webServerHandleClient()
{
}

// ===================== HTML TEMPLATES (Bootstrap 5 - LiXee-Box style) =====================

const char HTTP_HEADER[] PROGMEM =
    "<head>"
    "<script type='text/javascript' src='web/js/jquery-min.js'></script>"
    "<script type='text/javascript' src='web/js/bootstrap.min.js'></script>"
    "<script type='text/javascript' src='web/js/functions.js'></script>"
    "<link href='web/css/bootstrap.min.css' rel='stylesheet' type='text/css' />"
    "<link href='web/css/style.css' rel='stylesheet' type='text/css' />"
    "<meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    " </head>";

const char HTTP_MENU[] PROGMEM =
    "<body>"
    "<nav class='navbar navbar-expand-lg navbar-light bg-light rounded'><div class='container-fluid'><a class='navbar-brand' href='/'>"
    "<div style='display:block-inline;float:left;'><img src='web/img/logo.png'> </div>"
    "<div style='float:left;display:block-inline;font-weight:bold;padding:18px 10px 10px 10px;'> ZiBridge</div>"
    "</a>"
    "<button class='navbar-toggler' type='button' data-bs-toggle='collapse' data-bs-target='#navbarNavDropdown' aria-controls='navbarNavDropdown' aria-expanded='false' aria-label='Toggle navigation'>"
    "<span class='navbar-toggler-icon'></span>"
    "</button>"
    "<div id='navbarNavDropdown' class='collapse navbar-collapse'>"
    "<ul class='navbar-nav mx-auto mb-2 mb-lg-0'>"
    "<li class='nav-item'>"
    "<a class='nav-link' href='/'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M8 4a.5.5 0 0 1 .5.5V6a.5.5 0 0 1-1 0V4.5A.5.5 0 0 1 8 4M3.732 5.732a.5.5 0 0 1 .707 0l1.06 1.06a.5.5 0 0 1-.706.708l-1.06-1.06a.5.5 0 0 1 0-.708M2 10a.5.5 0 0 1 .5-.5h1.5a.5.5 0 0 1 0 1H2.5A.5.5 0 0 1 2 10m9.5-.5a.5.5 0 0 0 0 1h1.5a.5.5 0 0 0 0-1zm-2.793-3.06a.5.5 0 0 0-.707-.708l-1.06 1.06a.5.5 0 0 0 .707.708zM8 10a2 2 0 1 1-4 0 2 2 0 0 1 4 0m-.5 0a1.5 1.5 0 1 0-3 0 1.5 1.5 0 0 0 3 0M1 13.5a.5.5 0 0 1 .5-.5h13a.5.5 0 0 1 0 1h-13a.5.5 0 0 1-.5-.5'/></svg>"
    "Status</a>"
    "</li>"
    "<li class='nav-item dropdown'>"
    "<a class='nav-link dropdown-toggle' href='#' id='navbarDropdown' role='button' data-bs-toggle='dropdown'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M9.405 1.05c-.413-1.4-2.397-1.4-2.81 0l-.1.34a1.464 1.464 0 0 1-2.105.872l-.31-.17c-1.283-.698-2.686.705-1.987 1.987l.169.311c.446.82.023 1.841-.872 2.105l-.34.1c-1.4.413-1.4 2.397 0 2.81l.34.1a1.464 1.464 0 0 1 .872 2.105l-.17.31c-.698 1.283.705 2.686 1.987 1.987l.311-.169a1.464 1.464 0 0 1 2.105.872l.1.34c.413 1.4 2.397 1.4 2.81 0l.1-.34a1.464 1.464 0 0 1 2.105-.872l.31.17c1.283.698 2.686-.705 1.987-1.987l-.169-.311a1.464 1.464 0 0 1 .872-2.105l.34-.1c1.4-.413 1.4-2.397 0-2.81l-.34-.1a1.464 1.464 0 0 1-.872-2.105l.17-.31c.698-1.283-.705-2.686-1.987-1.987l-.311.169a1.464 1.464 0 0 1-2.105-.872zM8 10.93a2.929 2.929 0 1 1 0-5.86 2.929 2.929 0 0 1 0 5.858z'/></svg>"
    "Config</a>"
    "<div class='dropdown-menu'>"
    "<a class='dropdown-item' href='configGeneral'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M8 4.754a3.246 3.246 0 1 0 0 6.492 3.246 3.246 0 0 0 0-6.492M5.754 8a2.246 2.246 0 1 1 4.492 0 2.246 2.246 0 0 1-4.492 0'/><path d='M9.796 1.343c-.527-1.79-3.065-1.79-3.592 0l-.094.319a.873.873 0 0 1-1.255.52l-.292-.16c-1.64-.892-3.433.902-2.54 2.541l.159.292a.873.873 0 0 1-.52 1.255l-.319.094c-1.79.527-1.79 3.065 0 3.592l.319.094a.873.873 0 0 1 .52 1.255l-.16.292c-.892 1.64.901 3.434 2.541 2.54l.292-.159a.873.873 0 0 1 1.255.52l.094.319c.527 1.79 3.065 1.79 3.592 0l.094-.319a.873.873 0 0 1 1.255-.52l.292.16c1.64.893 3.434-.902 2.54-2.541l-.159-.292a.873.873 0 0 1 .52-1.255l.319-.094c1.79-.527 1.79-3.065 0-3.592l-.319-.094a.873.873 0 0 1-.52-1.255l.16-.292c.893-1.64-.902-3.433-2.541-2.54l-.292.159a.873.873 0 0 1-1.255-.52zm-2.633.283c.246-.835 1.428-.835 1.674 0l.094.319a1.873 1.873 0 0 0 2.693 1.115l.291-.16c.764-.415 1.6.42 1.184 1.185l-.159.292a1.873 1.873 0 0 0 1.116 2.692l.318.094c.835.246.835 1.428 0 1.674l-.319.094a1.873 1.873 0 0 0-1.115 2.693l.16.291c.415.764-.42 1.6-1.185 1.184l-.291-.159a1.873 1.873 0 0 0-2.693 1.116l-.094.318c-.246.835-1.428.835-1.674 0l-.094-.319a1.873 1.873 0 0 0-2.692-1.115l-.292.16c-.764.415-1.6-.42-1.184-1.185l.159-.291A1.873 1.873 0 0 0 1.945 8.93l-.319-.094c-.835-.246-.835-1.428 0-1.674l.319-.094A1.873 1.873 0 0 0 3.06 4.377l-.16-.292c-.415-.764.42-1.6 1.185-1.184l.292.159a1.873 1.873 0 0 0 2.692-1.115z'/></svg>"
    "General</a>"
    "<a class='dropdown-item' href='configWiFi'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M15.384 6.115a.485.485 0 0 0-.047-.736A12.44 12.44 0 0 0 8 3C5.259 3 2.723 3.882.663 5.379a.485.485 0 0 0-.048.736.52.52 0 0 0 .668.05A11.45 11.45 0 0 1 8 4c2.507 0 4.827.802 6.716 2.164.205.148.49.13.668-.049'/><path d='M13.229 8.271a.482.482 0 0 0-.063-.745A9.46 9.46 0 0 0 8 6c-1.905 0-3.68.56-5.166 1.526a.48.48 0 0 0-.063.745.525.525 0 0 0 .652.065A8.46 8.46 0 0 1 8 7a8.46 8.46 0 0 1 4.576 1.336c.206.132.48.108.653-.065'/><path d='M11.025 10.073a.5.5 0 0 0-.065-.74A7.47 7.47 0 0 0 8 8.5a7.47 7.47 0 0 0-2.96.833.5.5 0 0 0-.065.74.5.5 0 0 0 .652.065A6.46 6.46 0 0 1 8 9.5a6.47 6.47 0 0 1 2.373.538.5.5 0 0 0 .652-.065'/><path d='M9.585 11.692a.5.5 0 0 0-.065-.74 4.5 4.5 0 0 0-3.04 0 .5.5 0 0 0-.066.74.5.5 0 0 0 .652.065 3.5 3.5 0 0 1 1.868 0 .5.5 0 0 0 .65-.065M8 16a1 1 0 1 0 0-2 1 1 0 0 0 0 2'/></svg>"
    "WiFi</a>"
#ifndef ESP32S3
    "<a class='dropdown-item' href='configEthernet'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M14 13.5v-8A1.5 1.5 0 0 0 12.5 4H11V2a1 1 0 0 0-1-1H6a1 1 0 0 0-1 1v2H3.5A1.5 1.5 0 0 0 2 5.5v8A1.5 1.5 0 0 0 3.5 15h9a1.5 1.5 0 0 0 1.5-1.5M6 2h4v2H6zm2 11.5a.5.5 0 0 1-.5-.5V9h-1v1a.5.5 0 0 1-1 0V9h-1v2a.5.5 0 0 1-1 0V8.5a.5.5 0 0 1 .5-.5h7a.5.5 0 0 1 .5.5V11a.5.5 0 0 1-1 0V9h-1v1a.5.5 0 0 1-1 0V9H8.5v4a.5.5 0 0 1-.5.5'/></svg>"
    "Ethernet</a>"
#endif
    "</div>"
    "</li>"
    "<li class='nav-item'>"
    "<a class='nav-link' href='/tools'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M.102 2.223A3.004 3.004 0 0 0 3.78 5.897l6.341 6.252A3.003 3.003 0 0 0 13 16a3 3 0 1 0-.851-5.878L5.897 3.781A3.004 3.004 0 0 0 2.223.1l2.141 2.142L4 4l-1.757.364zm13.37 9.019.528.026.287.445.445.287.026.529L15 13l-.242.471-.026.529-.445.287-.287.445-.529.026L13 15l-.471-.242-.529-.026-.287-.445-.445-.287-.026-.529L11 13l.242-.471.026-.529.445-.287.287-.445.529-.026L13 11z'/></svg>"
    "Tools</a>"
    "</li>"
    "<li class='nav-item'>"
    "<a class='nav-link' href='/help'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M8 15A7 7 0 1 1 8 1a7 7 0 0 1 0 14m0 1A8 8 0 1 0 8 0a8 8 0 0 0 0 16'/><path d='M5.255 5.786a.237.237 0 0 0 .241.247h.825c.138 0 .248-.113.266-.25.09-.656.54-1.134 1.342-1.134.686 0 1.314.343 1.314 1.168 0 .635-.374.927-.965 1.371-.673.489-1.206 1.06-1.168 1.987l.003.217a.25.25 0 0 0 .25.246h.811a.25.25 0 0 0 .25-.25v-.105c0-.718.273-.927 1.01-1.486.609-.463 1.244-.977 1.244-2.056 0-1.511-1.276-2.241-2.673-2.241-1.267 0-2.655.59-2.75 2.286m1.557 5.763c0 .533.425.927 1.01.927.609 0 1.028-.394 1.028-.927 0-.552-.42-.94-1.029-.94-.584 0-1.009.388-1.009.94'/></svg>"
    "Help</a>"
    "</li>"
    "</ul></div></div>"
    "</nav>"
    "<div id='alert' style='display:none;' class='alert alert-success' role='alert'>"
    "</div>"
    "<div class='content-wrapper'>";

const char HTTP_FOOTER[] PROGMEM =
    "</div>"  // close content-wrapper
    "<footer class='text-center text-muted py-3' style='font-size:0.8rem;'>"
    "&copy; LiXee - ZiBridge " VERSION
    "</footer>"
    "</body>";

// ===================== CONFIG WIFI (exact LiXee-Box style) =====================

const char HTTP_CONFIG_WIFI[] PROGMEM =
    "<div class='page-title'>"
    "<div class='title-icon icon-blue'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M15.384 6.115a.485.485 0 0 0-.047-.736A12.44 12.44 0 0 0 8 3C5.259 3 2.723 3.882.663 5.379a.485.485 0 0 0-.048.736.52.52 0 0 0 .668.05A11.45 11.45 0 0 1 8 4c2.507 0 4.827.802 6.716 2.164.205.148.49.13.668-.049'/><path d='M13.229 8.271a.482.482 0 0 0-.063-.745A9.46 9.46 0 0 0 8 6c-1.905 0-3.68.56-5.166 1.526a.48.48 0 0 0-.063.745.525.525 0 0 0 .652.065A8.46 8.46 0 0 1 8 7a8.46 8.46 0 0 1 4.576 1.336c.206.132.48.108.653-.065'/><path d='M11.025 10.073a.5.5 0 0 0-.065-.74A7.47 7.47 0 0 0 8 8.5a7.47 7.47 0 0 0-2.96.833.5.5 0 0 0-.065.74.5.5 0 0 0 .652.065A6.46 6.46 0 0 1 8 9.5a6.47 6.47 0 0 1 2.373.538.5.5 0 0 0 .652-.065'/><path d='M9.585 11.692a.5.5 0 0 0-.065-.74 4.5 4.5 0 0 0-3.04 0 .5.5 0 0 0-.066.74.5.5 0 0 0 .652.065 3.5 3.5 0 0 1 1.868 0 .5.5 0 0 0 .65-.065M8 16a1 1 0 1 0 0-2 1 1 0 0 0 0 2'/></svg>"
    "</div>"
    "<h1>Config WiFi</h1>"
    "</div>"
    "<div class='row justify-content-md-center'>"
    "<div class='col-sm-6'>"
    "<div class='card'>"
    "<div class='card-header'>"
    "<div class='card-icon icon-blue'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M15.384 6.115a.485.485 0 0 0-.047-.736A12.44 12.44 0 0 0 8 3C5.259 3 2.723 3.882.663 5.379a.485.485 0 0 0-.048.736.52.52 0 0 0 .668.05A11.45 11.45 0 0 1 8 4c2.507 0 4.827.802 6.716 2.164.205.148.49.13.668-.049'/><path d='M13.229 8.271a.482.482 0 0 0-.063-.745A9.46 9.46 0 0 0 8 6c-1.905 0-3.68.56-5.166 1.526a.48.48 0 0 0-.063.745.525.525 0 0 0 .652.065A8.46 8.46 0 0 1 8 7a8.46 8.46 0 0 1 4.576 1.336c.206.132.48.108.653-.065'/><path d='M11.025 10.073a.5.5 0 0 0-.065-.74A7.47 7.47 0 0 0 8 8.5a7.47 7.47 0 0 0-2.96.833.5.5 0 0 0-.065.74.5.5 0 0 0 .652.065A6.46 6.46 0 0 1 8 9.5a6.47 6.47 0 0 1 2.373.538.5.5 0 0 0 .652-.065'/><path d='M9.585 11.692a.5.5 0 0 0-.065-.74 4.5 4.5 0 0 0-3.04 0 .5.5 0 0 0-.066.74.5.5 0 0 0 .652.065 3.5 3.5 0 0 1 1.868 0 .5.5 0 0 0 .65-.065M8 16a1 1 0 1 0 0-2 1 1 0 0 0 0 2'/></svg>"
    "</div>WiFi Settings"
    "</div>"
    "<div class='card-body config-form'>"
    "<form method='POST' action='saveWifi'>"
    "<div class='form-check'>"
    "<input class='form-check-input' id='wifiEnable' type='checkbox' name='wifiEnable' {{checkedWiFi}}>"
    "<label class='form-check-label' for='wifiEnable'>Enable</label>"
    "</div>"
    "<div class='form-group'>"
    "<label for='ssid'>SSID</label>"
    "<input class='form-control' id='ssid' type='text' name='WIFISSID' value='{{ssid}}'>"
    "<a onclick='scanNetwork();' class='btn btn-primary btn-sm mt-2'>"
    "<svg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='currentColor' viewBox='0 0 16 16'><path d='M11.742 10.344a6.5 6.5 0 1 0-1.397 1.398h-.001q.044.06.098.115l3.85 3.85a1 1 0 0 0 1.415-1.414l-3.85-3.85a1 1 0 0 0-.115-.1zM12 6.5a5.5 5.5 0 1 1-11 0 5.5 5.5 0 0 1 11 0'/></svg>"
    " Scan</a>"
    "<div id='networks'></div>"
    "</div>"
    "<div class='form-group'>"
    "<label for='pass'>Password</label>"
    "<input class='form-control' id='pass' type='password' name='WIFIpassword' value=''>"
    "</div>"
    "<div class='form-group'>"
    "<label for='ip'>@IP</label>"
    "<input class='form-control' id='ip' type='text' name='ipAddress' value='{{ip}}'>"
    "</div>"
    "<div class='form-group'>"
    "<label for='mask'>@Mask</label>"
    "<input class='form-control' id='mask' type='text' name='ipMask' value='{{mask}}'>"
    "</div>"
    "<div class='form-group'>"
    "<label for='gateway'>@Gateway</label>"
    "<input type='text' class='form-control' id='gateway' name='ipGW' value='{{gw}}'>"
    "</div>"
    "<button type='submit' class='btn btn-primary mb-2'name='save'>"
    "<svg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='currentColor' viewBox='0 0 16 16'><path d='M2 1a1 1 0 0 0-1 1v12a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1V2a1 1 0 0 0-1-1H9.5a1 1 0 0 0-1 1v7.293l2.646-2.647a.5.5 0 0 1 .708.708l-3.5 3.5a.5.5 0 0 1-.708 0l-3.5-3.5a.5.5 0 1 1 .708-.708L7.5 9.293V2a2 2 0 0 1 2-2H14a2 2 0 0 1 2 2v12a2 2 0 0 1-2 2H2a2 2 0 0 1-2-2V2a2 2 0 0 1 2-2h2.5a.5.5 0 0 1 0 1z'/></svg>"
    " Save</button>"
    "</form>"
    "</div></div></div></div>";

// ===================== CONFIG ETHERNET =====================

#ifndef ESP32S3
const char HTTP_ETHERNET[] PROGMEM =
    "<div class='page-title'>"
    "<div class='title-icon icon-teal'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M14 13.5v-8A1.5 1.5 0 0 0 12.5 4H11V2a1 1 0 0 0-1-1H6a1 1 0 0 0-1 1v2H3.5A1.5 1.5 0 0 0 2 5.5v8A1.5 1.5 0 0 0 3.5 15h9a1.5 1.5 0 0 0 1.5-1.5M6 2h4v2H6zm2 11.5a.5.5 0 0 1-.5-.5V9h-1v1a.5.5 0 0 1-1 0V9h-1v2a.5.5 0 0 1-1 0V8.5a.5.5 0 0 1 .5-.5h7a.5.5 0 0 1 .5.5V11a.5.5 0 0 1-1 0V9h-1v1a.5.5 0 0 1-1 0V9H8.5v4a.5.5 0 0 1-.5.5'/></svg>"
    "</div>"
    "<h1>Config Ethernet</h1>"
    "</div>"
    "<div class='row justify-content-md-center'>"
    "<div class='col-sm-6'>"
    "<div class='card'>"
    "<div class='card-header'>"
    "<div class='card-icon icon-teal'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M14 13.5v-8A1.5 1.5 0 0 0 12.5 4H11V2a1 1 0 0 0-1-1H6a1 1 0 0 0-1 1v2H3.5A1.5 1.5 0 0 0 2 5.5v8A1.5 1.5 0 0 0 3.5 15h9a1.5 1.5 0 0 0 1.5-1.5'/></svg>"
    "</div>Ethernet Settings"
    "</div>"
    "<div class='card-body config-form'>"
    "<form method='POST' action='saveEther'>"
    "<div class='form-check'>"
    "<input class='form-check-input' id='dhcp' type='checkbox' name='dhcp' {{modeEther}}>"
    "<label class='form-check-label' for='dhcp'>DHCP</label>"
    "</div>"
    "<div class='form-group'>"
    "<label for='ip'>@IP</label>"
    "<input class='form-control' id='ip' type='text' name='ipAddress' value='{{ipEther}}'>"
    "</div>"
    "<div class='form-group'>"
    "<label for='mask'>@Mask</label>"
    "<input class='form-control' id='mask' type='text' name='ipMask' value='{{maskEther}}'>"
    "</div>"
    "<div class='form-group'>"
    "<label for='gateway'>@Gateway</label>"
    "<input type='text' class='form-control' id='gateway' name='ipGW' value='{{GWEther}}'>"
    "</div>"
    "<div class='form-group'>"
    "<label>Server Port</label>"
    "<div class='form-control-plaintext'><strong>{{port}}</strong></div>"
    "</div>"
    "<button type='submit' class='btn btn-primary mb-2'name='save'>"
    "<svg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='currentColor' viewBox='0 0 16 16'><path d='M2 1a1 1 0 0 0-1 1v12a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1V2a1 1 0 0 0-1-1H9.5a1 1 0 0 0-1 1v7.293l2.646-2.647a.5.5 0 0 1 .708.708l-3.5 3.5a.5.5 0 0 1-.708 0l-3.5-3.5a.5.5 0 1 1 .708-.708L7.5 9.293V2a2 2 0 0 1 2-2H14a2 2 0 0 1 2 2v12a2 2 0 0 1-2 2H2a2 2 0 0 1-2-2V2a2 2 0 0 1 2-2h2.5a.5.5 0 0 1 0 1z'/></svg>"
    " Save</button>"
    "</form>"
    "</div></div></div></div>";
#endif

// ===================== GENERAL =====================

const char HTTP_GENERAL[] PROGMEM =
    "<div class='page-title'>"
    "<div class='title-icon icon-green'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M9.405 1.05c-.413-1.4-2.397-1.4-2.81 0l-.1.34a1.464 1.464 0 0 1-2.105.872l-.31-.17c-1.283-.698-2.686.705-1.987 1.987l.169.311c.446.82.023 1.841-.872 2.105l-.34.1c-1.4.413-1.4 2.397 0 2.81l.34.1a1.464 1.464 0 0 1 .872 2.105l-.17.31c-.698 1.283.705 2.686 1.987 1.987l.311-.169a1.464 1.464 0 0 1 2.105.872l.1.34c.413 1.4 2.397 1.4 2.81 0l.1-.34a1.464 1.464 0 0 1 2.105-.872l.31.17c1.283.698 2.686-.705 1.987-1.987l-.169-.311a1.464 1.464 0 0 1 .872-2.105l.34-.1c1.4-.413 1.4-2.397 0-2.81l-.34-.1a1.464 1.464 0 0 1-.872-2.105l.17-.31c.698-1.283-.705-2.686-1.987-1.987l-.311.169a1.464 1.464 0 0 1-2.105-.872zM8 10.93a2.929 2.929 0 1 1 0-5.86 2.929 2.929 0 0 1 0 5.858z'/></svg>"
    "</div>"
    "<h1>General</h1>"
    "</div>"
    "<div class='row justify-content-md-center'>"
    "<div class='col-sm-6'>"
    "<div class='card'>"
    "<div class='card-header'>"
    "<div class='card-icon icon-green'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M8 4.754a3.246 3.246 0 1 0 0 6.492 3.246 3.246 0 0 0 0-6.492'/></svg>"
    "</div>General Settings"
    "</div>"
    "<div class='card-body config-form'>"
    "<form method='POST' action='saveGeneral'>"
    "<div class='form-check'>"
    "<input class='form-check-input' id='disableWeb' type='checkbox' name='disableWeb' {{disableWeb}}>"
    "<label class='form-check-label' for='disableWeb'>Disable web server when ZiGate is connected</label>"
    "</div>"
    "<div class='form-check'>"
    "<input class='form-check-input' id='enableHeartBeat' type='checkbox' name='enableHeartBeat' {{enableHeartBeat}}>"
    "<label class='form-check-label' for='enableHeartBeat'>Enable HeartBeat (send ping to TCP when no trafic)</label>"
    "</div>"
    "<div class='form-group'>"
    "<label for='refreshLogs'>Refresh console log (ms)</label>"
    "<input class='form-control' id='refreshLogs' type='text' name='refreshLogs' value='{{refreshLogs}}'>"
    "</div>"
    "<button type='submit' class='btn btn-primary mb-2' name='save'>"
    "<svg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='currentColor' viewBox='0 0 16 16'><path d='M2 1a1 1 0 0 0-1 1v12a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1V2a1 1 0 0 0-1-1H9.5a1 1 0 0 0-1 1v7.293l2.646-2.647a.5.5 0 0 1 .708.708l-3.5 3.5a.5.5 0 0 1-.708 0l-3.5-3.5a.5.5 0 1 1 .708-.708L7.5 9.293V2a2 2 0 0 1 2-2H14a2 2 0 0 1 2 2v12a2 2 0 0 1-2 2H2a2 2 0 0 1-2-2V2a2 2 0 0 1 2-2h2.5a.5.5 0 0 1 0 1z'/></svg>"
    " Save</button>"
    "</form>"
    "</div></div></div></div>";

// ===================== HELP =====================

const char HTTP_HELP[] PROGMEM =
    "<div class='page-title'>"
    "<div class='title-icon icon-indigo'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M8 15A7 7 0 1 1 8 1a7 7 0 0 1 0 14m0 1A8 8 0 1 0 8 0a8 8 0 0 0 0 16'/><path d='M5.255 5.786a.237.237 0 0 0 .241.247h.825c.138 0 .248-.113.266-.25.09-.656.54-1.134 1.342-1.134.686 0 1.314.343 1.314 1.168 0 .635-.374.927-.965 1.371-.673.489-1.206 1.06-1.168 1.987l.003.217a.25.25 0 0 0 .25.246h.811a.25.25 0 0 0 .25-.25v-.105c0-.718.273-.927 1.01-1.486.609-.463 1.244-.977 1.244-2.056 0-1.511-1.276-2.241-2.673-2.241-1.267 0-2.655.59-2.75 2.286m1.557 5.763c0 .533.425.927 1.01.927.609 0 1.028-.394 1.028-.927 0-.552-.42-.94-1.029-.94-.584 0-1.009.388-1.009.94'/></svg>"
    "</div>"
    "<h1>Help</h1>"
    "</div>"
    "<div class='row' style='--bs-gutter-x: 0.5rem;'>"
    "<div class='col-sm-6'>"
    "<a href='https://lixee.fr/' target='_blank' class='tool-card' style='width:100%;display:block;'>"
    "<div class='tool-icon icon-green'>"
    "<svg xmlns='http://www.w3.org/2000/svg' width='28' height='28' fill='currentColor' viewBox='0 0 16 16'>"
    "<path d='M2.97 1.35A1 1 0 0 1 3.73 1h8.54a1 1 0 0 1 .76.35l2.609 3.044A1.5 1.5 0 0 1 16 5.37v.255a2.375 2.375 0 0 1-4.25 1.458A2.37 2.37 0 0 1 9.875 8 2.37 2.37 0 0 1 8 7.083 2.37 2.37 0 0 1 6.125 8a2.37 2.37 0 0 1-1.875-.917A2.375 2.375 0 0 1 0 5.625V5.37a1.5 1.5 0 0 1 .361-.976zm1.78 4.275a1.375 1.375 0 0 0 2.75 0 .5.5 0 0 1 1 0 1.375 1.375 0 0 0 2.75 0 .5.5 0 0 1 1 0 1.375 1.375 0 1 0 2.75 0V5.37a.5.5 0 0 0-.12-.325L12.27 2H3.73L1.12 5.045A.5.5 0 0 0 1 5.37v.255a1.375 1.375 0 0 0 2.75 0 .5.5 0 0 1 1 0M1.5 8.5A.5.5 0 0 1 2 9v6h12V9a.5.5 0 0 1 1 0v6h.5a.5.5 0 0 1 0 1H.5a.5.5 0 0 1 0-1H1V9a.5.5 0 0 1 .5-.5m2 .5a.5.5 0 0 1 .5.5V13h8V9.5a.5.5 0 0 1 1 0V13a1 1 0 0 1-1 1H4a1 1 0 0 1-1-1V9.5a.5.5 0 0 1 .5-.5'/>"
    "</svg></div>"
    "<div class='tool-name'>Shop &amp; description</div>"
    "<div class='tool-desc'>lixee.fr</div>"
    "</a>"
    "</div>"
    "<div class='col-sm-6'>"
    "<a href='https://github.com/fairecasoimeme/LiXee-ZiBridge' target='_blank' class='tool-card' style='width:100%;display:block;'>"
    "<div class='tool-icon icon-purple'>"
    "<svg xmlns='http://www.w3.org/2000/svg' width='28' height='28' fill='currentColor' viewBox='0 0 16 16'>"
    "<path d='M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27s1.36.09 2 .27c1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.01 8.01 0 0 0 16 8c0-4.42-3.58-8-8-8'/>"
    "</svg></div>"
    "<div class='tool-name'>Firmware Source &amp; Issues</div>"
    "<div class='tool-desc'>GitHub</div>"
    "</a>"
    "</div>"
    "</div>";

// ===================== TOOLS (LiXee-Box style) =====================

const char HTTP_TOOLS[] PROGMEM =
    "<div class='page-title'>"
    "<div class='title-icon icon-orange'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M.102 2.223A3.004 3.004 0 0 0 3.78 5.897l6.341 6.252A3.003 3.003 0 0 0 13 16a3 3 0 1 0-.851-5.878L5.897 3.781A3.004 3.004 0 0 0 2.223.1l2.141 2.142L4 4l-1.757.364zm13.37 9.019.528.026.287.445.445.287.026.529L15 13l-.242.471-.026.529-.445.287-.287.445-.529.026L13 15l-.471-.242-.529-.026-.287-.445-.445-.287-.026-.529L11 13l.242-.471.026-.529.445-.287.287-.445.529-.026L13 11z'/></svg>"
    "</div>"
    "<h1>Outils</h1>"
    "</div>"

    "<div class='tools-section'>"
    "<h6>Fichiers &amp; données</h6>"

    "<a href='/fsbrowser' class='tool-card'>"
    "<div class='tool-icon icon-blue'>"
    "<svg xmlns='http://www.w3.org/2000/svg' width='28' height='28' fill='currentColor' viewBox='0 0 16 16'>"
    "<path d='M9.293 0H4a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h8a2 2 0 0 0 2-2V4.707A1 1 0 0 0 13.707 4L10 .293A1 1 0 0 0 9.293 0M9.5 3.5v-2l3 3h-2a1 1 0 0 1-1-1M4.5 9a.5.5 0 0 1 0-1h7a.5.5 0 0 1 0 1zM4 10.5a.5.5 0 0 1 .5-.5h7a.5.5 0 0 1 0 1h-7a.5.5 0 0 1-.5-.5m.5 2.5a.5.5 0 0 1 0-1h4a.5.5 0 0 1 0 1z'/>"
    "</svg></div>"
    "<div class='tool-name'>Paramètres</div>"
    "<div class='tool-desc'>Fichiers devices</div>"
    "</a>"

    "<a href='/update' class='tool-card'>"
    "<div class='tool-icon icon-orange'>"
    "<svg xmlns='http://www.w3.org/2000/svg' width='28' height='28' fill='currentColor' viewBox='0 0 16 16'>"
    "<path d='M.5 9.9a.5.5 0 0 1 .5.5v2.5a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1v-2.5a.5.5 0 0 1 1 0v2.5a2 2 0 0 1-2 2H2a2 2 0 0 1-2-2v-2.5a.5.5 0 0 1 .5-.5'/>"
    "<path d='M7.646 11.854a.5.5 0 0 0 .708 0l3-3a.5.5 0 0 0-.708-.708L8.5 10.293V1.5a.5.5 0 0 0-1 0v8.793L5.354 8.146a.5.5 0 1 0-.708.708z'/>"
    "</svg></div>"
    "<div class='tool-name'>Mise à jour</div>"
    "<div class='tool-desc'>Update firmware</div>"
    "</a>"

    "</div>"

    "<div class='tools-section'>"
    "<h6>Système</h6>"

    "<a href='/logs' class='tool-card'>"
    "<div class='tool-icon icon-cyan'>"
    "<svg xmlns='http://www.w3.org/2000/svg' width='28' height='28' fill='currentColor' viewBox='0 0 16 16'>"
    "<path d='M6 9a.5.5 0 0 1 .5-.5h3a.5.5 0 0 1 0 1h-3A.5.5 0 0 1 6 9M2 2a2 2 0 0 1 2-2h8a2 2 0 0 1 2 2v12a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2zm10-1H4a1 1 0 0 0-1 1v12a1 1 0 0 0 1 1h8a1 1 0 0 0 1-1V2a1 1 0 0 0-1-1'/>"
    "</svg></div>"
    "<div class='tool-name'>Debug</div>"
    "<div class='tool-desc'>Console logs</div>"
    "</a>"

    "<a href='/reboot' class='tool-card'>"
    "<div class='tool-icon icon-pink'>"
    "<svg xmlns='http://www.w3.org/2000/svg' width='28' height='28' fill='currentColor' viewBox='0 0 16 16'>"
    "<path fill-rule='evenodd' d='M8 3a5 5 0 1 0 4.546 2.914.5.5 0 0 1 .908-.417A6 6 0 1 1 8 2z'/>"
    "<path d='M8 4.466V.534a.25.25 0 0 1 .41-.192l2.36 1.966c.12.1.12.284 0 .384L8.41 4.658A.25.25 0 0 1 8 4.466'/>"
    "</svg></div>"
    "<div class='tool-name'>Redémarrer</div>"
    "<div class='tool-desc'>Reboot système</div>"
    "</a>"

    "</div>";

// ===================== UPDATE (LiXee-Box style) =====================

const char HTTP_UPDATE[] PROGMEM =
    "<div class='page-title'>"
    "<div class='title-icon icon-orange'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M.5 9.9a.5.5 0 0 1 .5.5v2.5a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1v-2.5a.5.5 0 0 1 1 0v2.5a2 2 0 0 1-2 2H2a2 2 0 0 1-2-2v-2.5a.5.5 0 0 1 .5-.5'/><path d='M7.646 11.854a.5.5 0 0 0 .708 0l3-3a.5.5 0 0 0-.708-.708L8.5 10.293V1.5a.5.5 0 0 0-1 0v8.793L5.354 8.146a.5.5 0 1 0-.708.708z'/></svg>"
    "</div>"
    "<h1>Update firmware</h1>"
    "</div>"
    "<div class='card mb-3'>"
    "<div class='card-header'>"
    "<div class='card-icon icon-green'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27s1.36.09 2 .27c1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.01 8.01 0 0 0 16 8c0-4.42-3.58-8-8-8'/></svg>"
    "</div>Latest version on GitHub"
    "</div>"
    "<div class='card-body'>"
    "<div id='onlineupdate'>"
    "<h5 id=releasehead></h5>"
    "<pre id=releasebody style='white-space:pre-wrap;font-size:13px;background:#f8f9fa;padding:12px;border-radius:8px;'>Getting update information from GitHub...</pre>"
    "</div>"
    "<a class='btn btn-success mt-2' href='https://github.com/fairecasoimeme/LiXee-ZiBridge/releases/latest/download/lixee-zibridge.bin' id='downloadupdate' target='_blank'>"
    "<svg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='currentColor' viewBox='0 0 16 16'><path d='M.5 9.9a.5.5 0 0 1 .5.5v2.5a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1v-2.5a.5.5 0 0 1 1 0v2.5a2 2 0 0 1-2 2H2a2 2 0 0 1-2-2v-2.5a.5.5 0 0 1 .5-.5'/><path d='M7.646 11.854a.5.5 0 0 0 .708 0l3-3a.5.5 0 0 0-.708-.708L8.5 10.293V1.5a.5.5 0 0 0-1 0v8.793L5.354 8.146a.5.5 0 1 0-.708.708z'/></svg>"
    " Download</a>"
    "</div></div>"
    "<div class='card'>"
    "<div class='card-header'>"
    "<div class='card-icon icon-orange'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M.5 9.9a.5.5 0 0 1 .5.5v2.5a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1v-2.5a.5.5 0 0 1 1 0v2.5a2 2 0 0 1-2 2H2a2 2 0 0 1-2-2v-2.5a.5.5 0 0 1 .5-.5'/><path d='M7.646 11.854a.5.5 0 0 0 .708 0l3-3a.5.5 0 0 0-.708-.708L8.5 10.293V1.5a.5.5 0 0 0-1 0v8.793L5.354 8.146a.5.5 0 1 0-.708.708z'/></svg>"
    "</div>Upload firmware"
    "</div>"
    "<div class='card-body'>"
    "<form method='POST' action='/doUpdate' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update' id='file' onchange='sub(this)' style=display:none accept='.bin'>"
    "<label id='file-input' for='file'>   Choose file...</label>"
    "<input type='submit' class='btn btn-warning mb-2' value='Update'>"
    "<br><br>"
    "<div id='prg'></div>"
    "<br><div id='prgbar'><div id='bar'></div></div><br></form>"
    "</div></div>"
    "<script language='javascript'>getLatestReleaseInfo();</script>"
    "<script>"
    "function sub(obj){"
    "var fileName = obj.value.split('\\\\');"
    "document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
    "};"
    "$('form').submit(function(e){"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    "$.ajax({"
    "url: '/doUpdate',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData:false,"
    "xhr: function() {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener('progress', function(evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
    "$('#bar').css('width',Math.round(per*100) + '%');"
    "}"
    "}, false);"
    "return xhr;"
    "},"
    "success:function(d, s) {"
    "console.log('success!');"
    "$('#prg').html('Update completed!<br>Rebooting!');"
    "window.location.href='/';"
    "},"
    "error: function (a, b, c) {"
    "}"
    "});"
    "});"
    "</script>";

// ===================== STATUS (root page) =====================

const char HTTP_ROOT[] PROGMEM =
    "<div class='page-title'>"
    "<div class='title-icon icon-cyan'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M8 4a.5.5 0 0 1 .5.5V6a.5.5 0 0 1-1 0V4.5A.5.5 0 0 1 8 4M3.732 5.732a.5.5 0 0 1 .707 0l1.06 1.06a.5.5 0 0 1-.706.708l-1.06-1.06a.5.5 0 0 1 0-.708M2 10a.5.5 0 0 1 .5-.5h1.5a.5.5 0 0 1 0 1H2.5A.5.5 0 0 1 2 10m9.5-.5a.5.5 0 0 0 0 1h1.5a.5.5 0 0 0 0-1zm-2.793-3.06a.5.5 0 0 0-.707-.708l-1.06 1.06a.5.5 0 0 0 .707.708zM8 10a2 2 0 1 1-4 0 2 2 0 0 1 4 0m-.5 0a1.5 1.5 0 1 0-3 0 1.5 1.5 0 0 0 3 0M1 13.5a.5.5 0 0 1 .5-.5h13a.5.5 0 0 1 0 1h-13a.5.5 0 0 1-.5-.5'/></svg>"
    "</div>"
    "<h1>Status</h1>"
    "</div>"
    "<div class='row' style='--bs-gutter-x: 0.5rem;'>"
    "<div class='col-sm-4'>"
    "<div class='card'>"
    "<div class='card-header'>"
    "<div class='card-icon icon-blue'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M15.384 6.115a.485.485 0 0 0-.047-.736A12.44 12.44 0 0 0 8 3C5.259 3 2.723 3.882.663 5.379a.485.485 0 0 0-.048.736.52.52 0 0 0 .668.05A11.45 11.45 0 0 1 8 4c2.507 0 4.827.802 6.716 2.164.205.148.49.13.668-.049'/><path d='M13.229 8.271a.482.482 0 0 0-.063-.745A9.46 9.46 0 0 0 8 6c-1.905 0-3.68.56-5.166 1.526a.48.48 0 0 0-.063.745.525.525 0 0 0 .652.065A8.46 8.46 0 0 1 8 7a8.46 8.46 0 0 1 4.576 1.336c.206.132.48.108.653-.065'/><path d='M11.025 10.073a.5.5 0 0 0-.065-.74A7.47 7.47 0 0 0 8 8.5a7.47 7.47 0 0 0-2.96.833.5.5 0 0 0-.065.74.5.5 0 0 0 .652.065A6.46 6.46 0 0 1 8 9.5a6.47 6.47 0 0 1 2.373.538.5.5 0 0 0 .652-.065'/><path d='M9.585 11.692a.5.5 0 0 0-.065-.74 4.5 4.5 0 0 0-3.04 0 .5.5 0 0 0-.066.74.5.5 0 0 0 .652.065 3.5 3.5 0 0 1 1.868 0 .5.5 0 0 0 .65-.065M8 16a1 1 0 1 0 0-2 1 1 0 0 0 0 2'/></svg>"
    "</div>WiFi"
    "</div>"
    "<div class='card-body'>"
    "<table class='table table-sm table-borderless mb-0'>"
    "<tr><td><strong>Enable</strong></td><td>{{enableWifi}}</td></tr>"
    "<tr><td><strong>Connected</strong></td><td>{{connectedWifi}}</td></tr>"
    "<tr><td><strong>SSID</strong></td><td>{{ssidWifi}}</td></tr>"
    "<tr><td><strong>@IP</strong></td><td>{{ipWifi}}</td></tr>"
    "<tr><td><strong>@Mask</strong></td><td>{{maskWifi}}</td></tr>"
    "<tr><td><strong>@GW</strong></td><td>{{GWWifi}}</td></tr>"
    "</table>"
    "</div></div></div>"
#ifndef ESP32S3
    "<div class='col-sm-4'>"
    "<div class='card'>"
    "<div class='card-header'>"
    "<div class='card-icon icon-teal'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M14 13.5v-8A1.5 1.5 0 0 0 12.5 4H11V2a1 1 0 0 0-1-1H6a1 1 0 0 0-1 1v2H3.5A1.5 1.5 0 0 0 2 5.5v8A1.5 1.5 0 0 0 3.5 15h9a1.5 1.5 0 0 0 1.5-1.5'/></svg>"
    "</div>Ethernet"
    "</div>"
    "<div class='card-body'>"
    "<table class='table table-sm table-borderless mb-0'>"
    "<tr><td><strong>Enable</strong></td><td>{{enableEther}}</td></tr>"
    "<tr><td><strong>Connected</strong></td><td>{{connectedEther}}</td></tr>"
    "<tr><td><strong>Mode</strong></td><td>{{modeEther}}</td></tr>"
    "<tr><td><strong>@IP</strong></td><td>{{ipEther}}</td></tr>"
    "<tr><td><strong>@Mask</strong></td><td>{{maskEther}}</td></tr>"
    "<tr><td><strong>@GW</strong></td><td>{{GWEther}}</td></tr>"
    "</table>"
    "</div></div></div>"
#endif
    "<div class='col-sm-4'>"
    "<div class='card'>"
    "<div class='card-header'>"
    "<div class='card-icon icon-purple'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M5 0a.5.5 0 0 1 .5.5V2h1V.5a.5.5 0 0 1 1 0V2h1V.5a.5.5 0 0 1 1 0V2h1V.5a.5.5 0 0 1 1 0V2A2.5 2.5 0 0 1 14 4.5h1.5a.5.5 0 0 1 0 1H14v1h1.5a.5.5 0 0 1 0 1H14v1h1.5a.5.5 0 0 1 0 1H14v1h1.5a.5.5 0 0 1 0 1H14A2.5 2.5 0 0 1 11.5 14v1.5a.5.5 0 0 1-1 0V14h-1v1.5a.5.5 0 0 1-1 0V14h-1v1.5a.5.5 0 0 1-1 0V14h-1v1.5a.5.5 0 0 1-1 0V14A2.5 2.5 0 0 1 2 11.5H.5a.5.5 0 0 1 0-1H2v-1H.5a.5.5 0 0 1 0-1H2v-1H.5a.5.5 0 0 1 0-1H2v-1H.5a.5.5 0 0 1 0-1H2A2.5 2.5 0 0 1 4.5 2V.5a.5.5 0 0 1 1 0M3 4.5A1.5 1.5 0 0 0 4.5 6h7A1.5 1.5 0 0 0 13 4.5 1.5 1.5 0 0 0 11.5 3h-7A1.5 1.5 0 0 0 3 4.5'/></svg>"
    "</div>System Infos"
    "</div>"
    "<div class='card-body'>"
    "<table class='table table-sm table-borderless mb-0'>"
    "<tr><td><strong>Firmware</strong></td><td>" VERSION "</td></tr>"
    "</table>"
    "</div></div></div>"
    "</div>";

// ===================== HANDLERS =====================

void handleNotFound(AsyncWebServerRequest *request) {
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += request->url();
  message += F("\nMethod: ");
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += request->args();
  message += F("\n");

  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }

  request->send(404, F("text/plain"), message);
}

void handleHelp(AsyncWebServerRequest *request) {
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_MENU);
  result += FPSTR(HTTP_HELP);
  result += FPSTR(HTTP_FOOTER);
  result += F("</html>");

  request->send(200, "text/html", result);
}

void handleGeneral(AsyncWebServerRequest *request) {
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_MENU);
  result += FPSTR(HTTP_GENERAL);
  result += FPSTR(HTTP_FOOTER);
  result += F("</html>");

  if (ConfigSettings.disableWeb) {
    result.replace("{{disableWeb}}", "checked");
  } else {
    result.replace("{{disableWeb}}", "");
  }

  if (ConfigSettings.enableHeartBeat) {
    result.replace("{{enableHeartBeat}}", "checked");
  } else {
    result.replace("{{enableHeartBeat}}", "");
  }

  result.replace("{{refreshLogs}}", (String)ConfigSettings.refreshLogs);

  request->send(200, "text/html", result);
}

// ===================== CONFIG WIFI (exact LiXee-Box handler) =====================

void handleConfigWifi(AsyncWebServerRequest *request) {
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_MENU);
  result += FPSTR(HTTP_CONFIG_WIFI);
  result += FPSTR(HTTP_FOOTER);
  result += F("</html>");

  if (ConfigSettings.enableWiFi) {
    result.replace("{{checkedWiFi}}", "Checked");
  } else {
    result.replace("{{checkedWiFi}}", "");
  }
  result.replace("{{ssid}}", String(ConfigSettings.ssid));
  result.replace("{{ip}}", ConfigSettings.ipAddressWiFi);
  result.replace("{{mask}}", ConfigSettings.ipMaskWiFi);
  result.replace("{{gw}}", ConfigSettings.ipGWWiFi);

  request->send(200, "text/html", result);
}

#ifndef ESP32S3
void handleEther(AsyncWebServerRequest *request) {
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_MENU);
  result += FPSTR(HTTP_ETHERNET);
  result += FPSTR(HTTP_FOOTER);
  result += F("</html>");

  if (ConfigSettings.dhcp) {
    result.replace("{{modeEther}}", "Checked");
  } else {
    result.replace("{{modeEther}}", "");
  }
  result.replace("{{ipEther}}", ConfigSettings.ipAddress);
  result.replace("{{maskEther}}", ConfigSettings.ipMask);
  result.replace("{{GWEther}}", ConfigSettings.ipGW);
  result.replace("{{port}}", String(ConfigSettings.tcpListenPort));

  request->send(200, "text/html", result);
}
#endif

// ===================== STATUS PAGE =====================

#ifndef ESP32S3
uint32_t readADC_Cal(int ADC_Raw)
{
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  return(esp_adc_cal_raw_to_voltage(ADC_Raw, &adc_chars));
}
#endif

void handleRoot(AsyncWebServerRequest *request) {
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_MENU);
  result += FPSTR(HTTP_ROOT);
  result += FPSTR(HTTP_FOOTER);
  result += F("</html>");

  if (ConfigSettings.enableWiFi) {
    result.replace("{{enableWifi}}", "<img src='/web/img/ok.png'>");
  } else {
    result.replace("{{enableWifi}}", "<img src='/web/img/nok.png'>");
  }
  if (WiFi.status() == WL_CONNECTED) {
    result.replace("{{connectedWifi}}", "<img src='/web/img/ok.png'>");
  } else {
    result.replace("{{connectedWifi}}", "<img src='/web/img/nok.png'>");
  }
  result.replace("{{ssidWifi}}", String(ConfigSettings.ssid));
  result.replace("{{ipWifi}}", ConfigSettings.ipAddressWiFi);
  result.replace("{{maskWifi}}", ConfigSettings.ipMaskWiFi);
  result.replace("{{GWWifi}}", ConfigSettings.ipGWWiFi);

#ifndef ESP32S3
  result.replace("{{enableEther}}", "<img src='/web/img/ok.png'>");
  if (ConfigSettings.dhcp) {
    result.replace("{{modeEther}}", "DHCP");
    result.replace("{{ipEther}}", ETH.localIP().toString());
    result.replace("{{maskEther}}", ETH.subnetMask().toString());
    result.replace("{{GWEther}}", ETH.gatewayIP().toString());
  } else {
    result.replace("{{modeEther}}", "STATIC");
    result.replace("{{ipEther}}", ConfigSettings.ipAddress);
    result.replace("{{maskEther}}", ConfigSettings.ipMask);
    result.replace("{{GWEther}}", ConfigSettings.ipGW);
  }
  if (ConfigSettings.connectedEther) {
    result.replace("{{connectedEther}}", "<img src='/web/img/ok.png'>");
  } else {
    result.replace("{{connectedEther}}", "<img src='/web/img/nok.png'>");
  }
#endif

  request->send(200, "text/html", result);
}

// ===================== SAVE HANDLERS =====================

void handleSaveGeneral(AsyncWebServerRequest *request)
{
  String StringConfig;
  String disableWeb;
  String enableHeartBeat;
  String refreshLogs;

  if (request->arg("disableWeb") == "on") {
    disableWeb = "1";
  } else {
    disableWeb = "0";
  }

  if (request->arg("enableHeartBeat") == "on") {
    enableHeartBeat = "1";
  } else {
    enableHeartBeat = "0";
  }

  if (request->arg("refreshLogs").toDouble() < 1000) {
    refreshLogs = "1000";
  } else {
    refreshLogs = request->arg("refreshLogs");
  }

  const char *path = "/config/configGeneral.json";

  StringConfig = "{\"disableWeb\":" + disableWeb + ",\"modeUSB\":0,\"enableHeartBeat\":" + enableHeartBeat + ",\"refreshLogs\":" + refreshLogs + "}";
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, StringConfig);

  File configFile = LittleFS.open(path, FILE_WRITE);
  if (!configFile) {
    DEBUG_PRINTLN(F("failed open"));
  } else {
    serializeJson(doc, configFile);
    configFile.close();
  }
  request->redirect("/");
  needReboot = true;
}

void handleSaveWifi(AsyncWebServerRequest *request)
{
  if (!request->hasArg("WIFISSID")) {
    request->send(500, "text/plain", "BAD ARGS");
    return;
  }

  String StringConfig;
  String enableWiFi;
  if (request->arg("wifiEnable") == "on") {
    enableWiFi = "1";
  } else {
    enableWiFi = "0";
  }
  String ssid = request->arg("WIFISSID");
  String pass = request->arg("WIFIpassword");
  String ipAddress = request->arg("ipAddress");
  String ipMask = request->arg("ipMask");
  String ipGW = request->arg("ipGW");

  const char *path = "/config/config.json";

  StringConfig = "{\"enableWiFi\":" + enableWiFi + ",\"ssid\":\"" + ssid + "\",\"pass\":\"" + pass + "\",\"ip\":\"" + ipAddress + "\",\"mask\":\"" + ipMask + "\",\"gw\":\"" + ipGW + "\"}";
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, StringConfig);

  File configFile = LittleFS.open(path, FILE_WRITE);
  if (!configFile) {
    DEBUG_PRINTLN(F("failed open"));
  } else {
    serializeJson(doc, configFile);
    configFile.close();
  }
  request->redirect("/");
  needReboot = true;
}

#ifndef ESP32S3
void handleSaveEther(AsyncWebServerRequest *request)
{
  if (!request->hasArg("ipAddress")) {
    request->send(500, "text/plain", "BAD ARGS");
    return;
  }

  String StringConfig;
  String dhcp;
  if (request->arg("dhcp") == "on") {
    dhcp = "1";
  } else {
    dhcp = "0";
  }
  String ipAddress = request->arg("ipAddress");
  String ipMask = request->arg("ipMask");
  String ipGW = request->arg("ipGW");

  const char *path = "/config/configEther.json";

  StringConfig = "{\"dhcp\":" + dhcp + ",\"ip\":\"" + ipAddress + "\",\"mask\":\"" + ipMask + "\",\"gw\":\"" + ipGW + "\"}";
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, StringConfig);

  File configFile = LittleFS.open(path, FILE_WRITE);
  if (!configFile) {
    DEBUG_PRINTLN(F("failed open"));
  } else {
    serializeJson(doc, configFile);
    configFile.close();
  }
  request->redirect("/");
  needReboot = true;
}
#endif

// ===================== TOOLS =====================

void handleTools(AsyncWebServerRequest *request)
{
  String result;
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_MENU);
  response->print(result);
  result = FPSTR(HTTP_TOOLS);
  result += FPSTR(HTTP_FOOTER);
  result += F("</html>");
  response->print(result);
  request->send(response);
}

// ===================== LOGS =====================

void handleLogs(AsyncWebServerRequest *request) {
  String result;

  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_MENU);

  result += F("<div class='page-title'>"
    "<div class='title-icon icon-cyan'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M6 9a.5.5 0 0 1 .5-.5h3a.5.5 0 0 1 0 1h-3A.5.5 0 0 1 6 9M2 2a2 2 0 0 1 2-2h8a2 2 0 0 1 2 2v12a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2zm10-1H4a1 1 0 0 0-1 1v12a1 1 0 0 0 1 1h8a1 1 0 0 0 1-1V2a1 1 0 0 0-1-1'/></svg>"
    "</div>"
    "<h1>Console</h1>"
    "</div>");

  result += F("<div class='action-btns'>");
  result += F("<button type='button' onclick='cmd(\"ClearConsole\");document.getElementById(\"console\").value=\"\";' class='btn btn-outline-danger btn-sm'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M5.5 5.5A.5.5 0 0 1 6 6v6a.5.5 0 0 1-1 0V6a.5.5 0 0 1 .5-.5m2.5 0a.5.5 0 0 1 .5.5v6a.5.5 0 0 1-1 0V6a.5.5 0 0 1 .5-.5m3 .5a.5.5 0 0 0-1 0v6a.5.5 0 0 0 1 0z'/><path d='M14.5 3a1 1 0 0 1-1 1H13v9a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V4h-.5a1 1 0 0 1-1-1V2a1 1 0 0 1 1-1H6a1 1 0 0 1 1-1h2a1 1 0 0 1 1 1h3.5a1 1 0 0 1 1 1zM4.118 4 4 4.059V13a1 1 0 0 0 1 1h6a1 1 0 0 0 1-1V4.059L11.882 4zM2.5 3h11V2h-11z'/></svg>"
    " Clear</button>");
  result += F("<button type='button' onclick='cmd(\"GetVersion\");' class='btn btn-outline-primary btn-sm'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M8 4a.5.5 0 0 1 .5.5V6a.5.5 0 0 1-1 0V4.5A.5.5 0 0 1 8 4M3.732 5.732a.5.5 0 0 1 .707 0l1.06 1.06a.5.5 0 0 1-.706.708l-1.06-1.06a.5.5 0 0 1 0-.708M2 10a.5.5 0 0 1 .5-.5h1.5a.5.5 0 0 1 0 1H2.5A.5.5 0 0 1 2 10m9.5-.5a.5.5 0 0 0 0 1h1.5a.5.5 0 0 0 0-1zm-2.793-3.06a.5.5 0 0 0-.707-.708l-1.06 1.06a.5.5 0 0 0 .707.708zM8 10a2 2 0 1 1-4 0 2 2 0 0 1 4 0m-.5 0a1.5 1.5 0 1 0-3 0 1.5 1.5 0 0 0 3 0M1 13.5a.5.5 0 0 1 .5-.5h13a.5.5 0 0 1 0 1h-13a.5.5 0 0 1-.5-.5'/></svg>"
    " Get Version</button>");
  result += F("<button type='button' onclick='cmd(\"ErasePDM\");' class='btn btn-outline-warning btn-sm'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M8.086 2.207a2 2 0 0 1 2.828 0l3.879 3.879a2 2 0 0 1 0 2.828l-5.5 5.5A2 2 0 0 1 7.879 15H5.12a2 2 0 0 1-1.414-.586l-2.5-2.5a2 2 0 0 1 0-2.828zm.66.71-5.5 5.5a1 1 0 0 0 0 1.414l2.5 2.5a1 1 0 0 0 .707.293H7.88a1 1 0 0 0 .707-.293l5.5-5.5a1 1 0 0 0 0-1.414l-3.879-3.879a1 1 0 0 0-1.414 0z'/></svg>"
    " Erase PDM</button>");
  result += F("</div>");

  result += F("<div class='card'>"
    "<div class='card-header'>"
    "<div class='card-icon icon-cyan'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M14 1a1 1 0 0 1 1 1v12a1 1 0 0 1-1 1H2a1 1 0 0 1-1-1V2a1 1 0 0 1 1-1zM2 0a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V2a2 2 0 0 0-2-2z'/><path d='M5 6a.5.5 0 0 1 .5-.5h5a.5.5 0 0 1 0 1h-5A.5.5 0 0 1 5 6m0 2a.5.5 0 0 1 .5-.5h5a.5.5 0 0 1 0 1h-5A.5.5 0 0 1 5 8m0 2a.5.5 0 0 1 .5-.5h3a.5.5 0 0 1 0 1h-3A.5.5 0 0 1 5 10'/></svg>"
    "</div>Raw data"
    "</div>"
    "<div class='card-body'>"
    "<textarea class='console-area' id='console' rows='20'></textarea>"
    "</div></div>");

  result += FPSTR(HTTP_FOOTER);
  result += F("<script language='javascript'>logRefresh();</script>");
  result += F("</html>");

  request->send(200, F("text/html"), result);
}

// ===================== FSBROWSER (LiXee-Box style) =====================

void handleFSbrowser(AsyncWebServerRequest *request)
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_MENU);

  result += F("<div class='page-title'>"
    "<div class='title-icon icon-blue'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M9.293 0H4a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h8a2 2 0 0 0 2-2V4.707A1 1 0 0 0 13.707 4L10 .293A1 1 0 0 0 9.293 0M9.5 3.5v-2l3 3h-2a1 1 0 0 1-1-1M4.5 9a.5.5 0 0 1 0-1h7a.5.5 0 0 1 0 1zM4 10.5a.5.5 0 0 1 .5-.5h7a.5.5 0 0 1 0 1h-7a.5.5 0 0 1-.5-.5m.5 2.5a.5.5 0 0 1 0-1h4a.5.5 0 0 1 0 1z'/></svg>"
    "</div>"
    "<h1>Devices list files</h1>"
    "</div>");

  result += F("<nav id='navbar-custom' class='navbar navbar-default navbar-fixed-left'>");
  result += F("<div class='navbar-header'></div>");
  result += F("<ul class='nav navbar-nav'>");

  File root = LittleFS.open("/config");
  File file = root.openNextFile();
  while (file)
  {
    if (!file.isDirectory())
    {
      String tmp = file.name();
      result += F("<li><a href='#' onClick=\"readfile('");
      result += tmp;
      result += F("','config');document.getElementById('actions').style.display='block';\">");
      result += tmp;
      result += F(" ( ");
      result += file.size();
      result += F(" o)</a></li>");
    }
    file.close();
    vTaskDelay(1);
    file = root.openNextFile();
  }
  result += F("</ul></nav>");
  result += F("<div class='container-fluid'>");
  result += F("<div class='app-main-content'>");
  result += F("<div class='card'>"
    "<div class='card-header'>"
    "<div class='card-icon icon-blue'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M14 4.5V14a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V2a2 2 0 0 1 2-2h5.5zm-3 0A1.5 1.5 0 0 1 9.5 3V1H4a1 1 0 0 0-1 1v12a1 1 0 0 0 1 1h8a1 1 0 0 0 1-1V4.5z'/></svg>"
    "</div>File : <span id='title'></span>"
    "</div>"
    "<div class='card-body'>");
  result += F("<form method='POST' action='saveFile'>");
  result += F("<input type='hidden' name='filename' id='filename' value=''>");
  result += F("<textarea class='form-control' id='file' name='file' rows='10' style='border-radius:8px;font-family:monospace;font-size:13px;'></textarea>");
  result += F("<div id='actions' style='display:none;margin-top:12px;'>");
  result += F("<button type='submit' class='btn btn-warning btn-sm' name='save' value='save'>"
    "<svg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='currentColor' viewBox='0 0 16 16'><path d='M2 1a1 1 0 0 0-1 1v12a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1V2a1 1 0 0 0-1-1H9.5a1 1 0 0 0-1 1v7.293l2.646-2.647a.5.5 0 0 1 .708.708l-3.5 3.5a.5.5 0 0 1-.708 0l-3.5-3.5a.5.5 0 1 1 .708-.708L7.5 9.293V2a2 2 0 0 1 2-2H14a2 2 0 0 1 2 2v12a2 2 0 0 1-2 2H2a2 2 0 0 1-2-2V2a2 2 0 0 1 2-2h2.5a.5.5 0 0 1 0 1z'/></svg>"
    " Save</button>&nbsp;");
  result += F("<button type='submit' class='btn btn-danger btn-sm' name='delete' value='delete' onClick=\"if(confirm('Are you sure ?')==true){return true;}else{return false;};\">"
    "<svg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='currentColor' viewBox='0 0 16 16'><path d='M5.5 5.5A.5.5 0 0 1 6 6v6a.5.5 0 0 1-1 0V6a.5.5 0 0 1 .5-.5m2.5 0a.5.5 0 0 1 .5.5v6a.5.5 0 0 1-1 0V6a.5.5 0 0 1 .5-.5m3 .5a.5.5 0 0 0-1 0v6a.5.5 0 0 0 1 0z'/><path d='M14.5 3a1 1 0 0 1-1 1H13v9a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V4h-.5a1 1 0 0 1-1-1V2a1 1 0 0 1 1-1H6a1 1 0 0 1 1-1h2a1 1 0 0 1 1 1h3.5a1 1 0 0 1 1 1zM4.118 4 4 4.059V13a1 1 0 0 0 1 1h6a1 1 0 0 0 1-1V4.059L11.882 4zM2.5 3h11V2h-11z'/></svg>"
    " Delete</button>");
  result += F("</div></form>");
  result += F("</div></div>");  // card-body, card
  result += F("</div></div>");  // app-main-content, container-fluid
  result += FPSTR(HTTP_FOOTER);
  result += F("</html>");
  file.close();
  root.close();
  request->send(200, F("text/html"), result);
}

void handleReadfile(AsyncWebServerRequest *request)
{
  String result;
  int i = 0;
  String repertory = request->arg(i);
  String filename = "/" + repertory + "/" + request->arg(1);
  DEBUG_PRINTLN(filename);
  File file = LittleFS.open(filename, "r");

  if (!file || file.isDirectory()) {
    file.close();
    return;
  }

  while (file.available()) {
    result += (char)file.read();
  }
  file.close();
  request->send(200, F("text/html"), result);
}

void handleSavefile(AsyncWebServerRequest *request)
{
  if (request->method() != HTTP_POST) {
    request->send(405, F("text/plain"), F("Method Not Allowed"));
  } else {
    uint8_t i = 0;
    String filename = "/config/" + request->arg(i);
    String content = request->arg(1);
    String action = request->arg(2);

    if (action == "save")
    {
      File file = LittleFS.open(filename, "w+");
      if (!file || file.isDirectory()) {
        DEBUG_PRINT(F("Failed to open file for reading\r\n"));
        file.close();
        return;
      }

      int bytesWritten = file.print(content);

      if (bytesWritten > 0) {
        DEBUG_PRINT(F("File was written : "));
        DEBUG_PRINTLN(bytesWritten);
      } else {
        DEBUG_PRINT(F("File write failed : "));
        DEBUG_PRINTLN(filename);
      }

      file.close();
    }
    else if (action == "delete")
    {
      LittleFS.remove(filename);
    }
    AsyncWebServerResponse *response = request->beginResponse(303);
    response->addHeader(F("Location"), F("/fsbrowser"));
    request->send(response);
  }
}

// ===================== REBOOT =====================

void hard_restart() {
  esp_task_wdt_init(1, true);
  esp_task_wdt_add(NULL);
  while (true);
}

void handleReboot(AsyncWebServerRequest *request) {
  request->redirect("/");
  needReboot = true;
}

// ===================== BACKUP =====================

void handleToolBackup(AsyncWebServerRequest *request)
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_MENU);

  result += F("<div class='page-title'>"
    "<div class='title-icon icon-purple'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M.5 9.9a.5.5 0 0 1 .5.5v2.5a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1v-2.5a.5.5 0 0 1 1 0v2.5a2 2 0 0 1-2 2H2a2 2 0 0 1-2-2v-2.5a.5.5 0 0 1 .5-.5'/><path d='M7.646 1.146a.5.5 0 0 1 .708 0l3 3a.5.5 0 0 1-.708.708L8.5 2.707V11.5a.5.5 0 0 1-1 0V2.707L5.354 4.854a.5.5 0 1 1-.708-.708z'/></svg>"
    "</div>"
    "<h1>Backup</h1>"
    "</div>");

  result += F("<div class='card'>"
    "<div class='card-header'>"
    "<div class='card-icon icon-purple'>"
    "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M.5 9.9a.5.5 0 0 1 .5.5v2.5a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1v-2.5a.5.5 0 0 1 1 0v2.5a2 2 0 0 1-2 2H2a2 2 0 0 1-2-2v-2.5a.5.5 0 0 1 .5-.5'/></svg>"
    "</div>Configuration files"
    "</div>"
    "<div class='card-body'>"
    "<ul class='file-list'>");

  File root = LittleFS.open("/config");
  File file = root.openNextFile();
  while (file)
  {
    if (!file.isDirectory())
    {
      String tmp = file.name();
      result += F("<li>"
        "<svg xmlns='http://www.w3.org/2000/svg' fill='currentColor' viewBox='0 0 16 16'><path d='M14 4.5V14a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V2a2 2 0 0 1 2-2h5.5zm-3 0A1.5 1.5 0 0 1 9.5 3V1H4a1 1 0 0 0-1 1v12a1 1 0 0 0 1 1h8a1 1 0 0 0 1-1V4.5z'/></svg>"
        "<a href='/downloadFile?file=");
      result += tmp;
      result += F("'>");
      result += tmp;
      result += F("</a><span class='file-size'>");
      result += file.size();
      result += F(" o</span></li>");
    }
    file.close();
    vTaskDelay(1);
    file = root.openNextFile();
  }
  root.close();
  file.close();

  result += F("</ul></div></div>");
  result += FPSTR(HTTP_FOOTER);
  result += F("</html>");

  request->send(200, F("text/html"), result);
}

void handleDownloadFile(AsyncWebServerRequest *request)
{
  String filename = "/config/" + request->arg("file");
  if (LittleFS.exists(filename)) {
    request->send(LittleFS, filename, "application/octet-stream");
  } else {
    request->send(404, "text/plain", "File not found");
  }
}

// ===================== UPDATE (LiXee-Box OTA system) =====================

void handleToolUpdate(AsyncWebServerRequest *request)
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_MENU);
  result += FPSTR(HTTP_UPDATE);
  result += FPSTR(HTTP_FOOTER);
  result += F("</html>");

  request->send(200, F("text/html"), result);
}

size_t content_len;
#define U_PART U_SPIFFS

void handleDoUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!index) {
    DEBUG_PRINTLN("Update");
    content_len = request->contentLength();
    int cmd = (filename.indexOf("spiffs") > -1) ? U_PART : U_FLASH;
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
      Update.printError(Serial);
    }
  }

  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  }

  if (final) {
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
    response->addHeader("Refresh", "20");
    response->addHeader("Location", "/");
    request->send(response);
    if (!Update.end(true)) {
      Update.printError(Serial);
    } else {
      DEBUG_PRINTLN("Update complete");
      Serial.flush();
      ESP.restart();
    }
  }
}

void printProgress(size_t prg, size_t sz) {
  Serial.printf("Progress: %d%%\n", (prg * 100) / content_len);
}

// ===================== SCAN NETWORK =====================

void handleScanStart(AsyncWebServerRequest *request)
{
  WiFi.scanDelete();
  // scanNetworks(async, show_hidden, passive, max_ms_per_chan)
  // passive=true avoids disconnecting the current STA connection
  WiFi.scanNetworks(true, false, true, 300);
  log_d("WiFi scan started (async, passive)");
  request->send(200, F("application/json"), "{\"status\":\"started\"}");
}

void handleScanResult(AsyncWebServerRequest *request)
{
  int n = WiFi.scanComplete();
  log_d("scanComplete() = %d", n);

  if (n == WIFI_SCAN_RUNNING || n == WIFI_SCAN_FAILED) {
    log_d("Scan not ready yet");
    request->send(200, F("application/json"), "{\"status\":\"scanning\"}");
    return;
  }

  // Scan complete — build JSON
  log_d("Scan done, %d networks found", n);
  String json = "{\"status\":\"done\",\"networks\":[";
  for (int i = 0; i < n; ++i) {
    if (i > 0) json += ",";
    String ssid = WiFi.SSID(i);
    ssid.replace("\\", "\\\\");
    ssid.replace("\"", "\\\"");
    json += "{\"ssid\":\"" + ssid + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    log_d("  [%d] %s (%d dBm)", i, ssid.c_str(), WiFi.RSSI(i));
  }
  json += "]}";
  WiFi.scanDelete();
  log_d("Sending JSON response: %d bytes", json.length());
  request->send(200, F("application/json"), json);
}

// ===================== LOG / CONSOLE COMMANDS =====================

void handleLogBuffer(AsyncWebServerRequest *request)
{
  String result;
  result = logPrint();
  request->send(200, F("text/html"), result);
}

void handleClearConsole(AsyncWebServerRequest *request)
{
  logClear();
  request->send(200, F("text/html"), "");
}

void handleGetVersion(AsyncWebServerRequest *request)
{
  char output_sprintf[2];
  uint8_t cmd[10];
  cmd[0] = 0x01;
  cmd[1] = 0x02;
  cmd[2] = 0x10;
  cmd[3] = 0x10;
  cmd[4] = 0x02;
  cmd[5] = 0x10;
  cmd[6] = 0x02;
  cmd[7] = 0x10;
  cmd[8] = 0x10;
  cmd[9] = 0x03;

  ZiGateSerial.write(cmd, 10);
  ZiGateSerial.flush();

  String tmpTime;
  String buff = "";
  timeLog = millis();
  tmpTime = String(timeLog, DEC);
  logPush('[');
  for (int j = 0; j < tmpTime.length(); j++) {
    logPush(tmpTime[j]);
  }
  logPush(']');
  logPush('-');
  logPush('>');

  for (int i = 0; i < 10; i++) {
    sprintf(output_sprintf, "%02x", cmd[i]);
    logPush(' ');
    logPush(output_sprintf[0]);
    logPush(output_sprintf[1]);
  }
  logPush('\n');
  request->send(200, F("text/html"), "");
}

// ===================== INIT WEB SERVER =====================

void initWebServer()
{
  // Static files with cache
  serverWeb.serveStatic("/web/js/jquery-min.js", LittleFS, "/web/js/jquery-min.js").setCacheControl("max-age=600");
  serverWeb.serveStatic("/web/js/functions.js", LittleFS, "/web/js/functions.js").setCacheControl("max-age=600");
  serverWeb.serveStatic("/web/js/bootstrap.min.js", LittleFS, "/web/js/bootstrap.min.js").setCacheControl("max-age=600");
  serverWeb.serveStatic("/web/js/bootstrap.min.js.map", LittleFS, "/web/js/bootstrap.min.js.map").setCacheControl("max-age=600");
  serverWeb.serveStatic("/web/css/bootstrap.min.css", LittleFS, "/web/css/bootstrap.min.css").setCacheControl("max-age=600");
  serverWeb.serveStatic("/web/css/style.css", LittleFS, "/web/css/style.css").setCacheControl("max-age=600");
  serverWeb.serveStatic("/web/img/logo.png", LittleFS, "/web/img/logo.png").setCacheControl("max-age=600");
  serverWeb.serveStatic("/web/img/wait.gif", LittleFS, "/web/img/wait.gif").setCacheControl("max-age=600");
  serverWeb.serveStatic("/web/img/nok.png", LittleFS, "/web/img/nok.png").setCacheControl("max-age=600");
  serverWeb.serveStatic("/web/img/ok.png", LittleFS, "/web/img/ok.png").setCacheControl("max-age=600");
  serverWeb.serveStatic("/web/img/", LittleFS, "/web/img/").setCacheControl("max-age=600");

  // Pages
  serverWeb.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleRoot(request);
  });
  serverWeb.on("/configGeneral", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleGeneral(request);
  });
  serverWeb.on("/configWiFi", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleConfigWifi(request);
  });
#ifndef ESP32S3
  serverWeb.on("/configEthernet", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleEther(request);
  });
#endif
  serverWeb.on("/saveGeneral", HTTP_POST, [](AsyncWebServerRequest *request) {
    handleSaveGeneral(request);
  });
  serverWeb.on("/saveWifi", HTTP_POST, [](AsyncWebServerRequest *request) {
    handleSaveWifi(request);
  });
#ifndef ESP32S3
  serverWeb.on("/saveEther", HTTP_POST, [](AsyncWebServerRequest *request) {
    handleSaveEther(request);
  });
#endif
  serverWeb.on("/tools", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleTools(request);
  });
  serverWeb.on("/fsbrowser", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleFSbrowser(request);
  });
  serverWeb.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleLogs(request);
  });
  serverWeb.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleReboot(request);
  });
  serverWeb.on("/backup", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleToolBackup(request);
  });
  serverWeb.on("/downloadFile", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleDownloadFile(request);
  });
  serverWeb.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleToolUpdate(request);
  });
  serverWeb.on("/doUpdate", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data,
       size_t len, bool final) {
      handleDoUpdate(request, filename, index, data, len, final);
    }
  );
  serverWeb.on("/readFile", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleReadfile(request);
  });
  serverWeb.on("/saveFile", HTTP_POST, [](AsyncWebServerRequest *request) {
    handleSavefile(request);
  });
  serverWeb.on("/getLogBuffer", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleLogBuffer(request);
  });
  serverWeb.on("/scanStart", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleScanStart(request);
  });
  serverWeb.on("/scanResult", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleScanResult(request);
  });
  serverWeb.on("/cmdClearConsole", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleClearConsole(request);
  });
  serverWeb.on("/cmdGetVersion", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleGetVersion(request);
  });
  serverWeb.on("/help", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleHelp(request);
  });
  serverWeb.onNotFound(handleNotFound);

  serverWeb.begin();
  Update.onProgress(printProgress);
}
