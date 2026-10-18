#ifndef __PAGES_H
#define __PAGES_H

// ثابت‌های HTML
const char DASHBOARD_HEAD[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard</title>
    <style>
      body { font-family: 'Segoe UI', sans-serif; background-color: #f9f9f9; margin: 0; padding: 0; }
      .section { background-color: #ffffff; border: 1px solid #ddd; border-radius: 6px; width: 90%; max-width: 800px; margin: 20px auto; box-shadow: 0 2px 5px rgba(0,0,0,0.05); overflow: hidden; }
      .container { display: flex; flex-wrap: wrap; justify-content: space-between; padding: 6px 10px; border-bottom: 1px solid #eee; background-color: #f8f8f8; }
      .container:last-child { border-bottom: none; }
      .cell {display:block; padding: 4px; font-size: 14px; line-height: 1.2; word-break: break-word; }
      a { text-decoration: none; color: #0077cc; margin-right: 10px; font-weight: bold; }  
      a:hover { text-decoration: underline; }
      .viewbtn{display:block; background-color: #68aa7dff; float:left; color:#fff; border-radius:4px; padding:6px 12px; }
      .downloadbtn{display:block; background-color: #324673ff; float:left; color:#fff; border-radius:4px; padding:6px 12px; }
      .deletebtn{display:block; background-color: #f43f0eff; float:left; color:#fff; border-radius:4px; padding:6px 12px; }
      form input, form select { padding: 6px; border-radius: 4px; border: 1px solid #ccc; width: 100%; box-sizing: border-box; font-size: 13px; }
      form input[type='submit'] { background-color: #0077cc; color: white; border: none; cursor: pointer; padding: 6px 12px; width: 100px; margin-top:4px; }
      @media screen and (max-width: 600px) {
        .cell { flex: 1 1 100%; padding: 6px 0; }
        form input, form select { font-size: 15px; }
      }
    </style>  
  </head>
<body>
  <div class='section'>
	<div class='container' style='font-weight:bold; background-color:#f0f0f0'>
		<div class='cell'>FILE NAME</div>
		<div class='cell'>FILE SIZE</div>
		<div class='cell'>ACTION</div>
    </div>
)rawliteral";

const char FORM_BLOCKS[] PROGMEM = R"rawliteral(
<div class='section'>

  <!-- Upload File -->
  <form method='POST' action='/handleupload' enctype='multipart/form-data'>
    <div class='container'>
      <div class='cell' style='flex:1 1 100%; font-weight:bold;'>Upload File</div>
      <div class='cell'><label>Folder</label>
        <select name='folder'>
          <option value='www'>www</option>
          <option value='data'>data</option>
          <option value='config'>config</option>
        </select>
      </div>
      <div class='cell'><label>File</label><input type='file' name='filename' required></div>
      <div class='cell'><input type='submit' value='Upload'></div>
    </div>
  </form>

  <!-- WiFi Config -->
  <form method='POST' action='/setwifi'>
    <div class='container'>
        <div class='cell' style='flex:1 1 100%; font-weight:bold;'>WiFi Config</div>
        <div class='cell'><label>SSID</label><input type='text' name='ssid' value='%SSID%' required></div>
        <div class='cell'><label>Password</label><input type='text' name='pass' value='%PASS%' required></div>
        <div class='cell'><label>Enable</label><input type='checkbox' name='wienbl' %WIENBL%></div>
        <div class='cell'><input type='submit' value='Save WiFi'></div>
    </div>
  </form>

  <!-- Firmware Update -->
  <form method='POST' action='/handleupdate' enctype='multipart/form-data'>
    <div class='container'>
      <div class='cell' style='flex:1 1 100%; font-weight:bold;'>Firmware Update</div>
      <div class='cell'><label>Firmware File</label><input type='file' name='filename' accept='.bin' required></div>
      <div class='cell'><input type='submit' value='Install'></div>
    </div>
  </form>

</div>
)rawliteral";

const char PARAMS_BLOCK[] PROGMEM = R"rawliteral(
<div class='section'>
  <div class='container' style='font-weight:bold; background-color:#f0f0f0'>
    <div class='cell'>Parameters</div><div class='cell'>Value</div>
  </div>

  <div class='container'>
    <div class='cell'>File System Usage</div>
    <div class='cell' style='flex:2'>
      <div style='background:#eee; border-radius:4px; height:28px; position:relative;'>
        <div style='background:#4caf50; height:100%; width:%USED%%; min-width:4px;'></div>
        <div style='position:absolute; top:0; left:0; width:100%; height:100%;
                    display:flex; align-items:center; justify-content:center;
                     font-weight:bold; color:#000; text-shadow:1px 1px 2px rgba(255,255,255,0.6);'>
          %USED%% (%USED_STR% / %TOTAL_STR%)
        </div>
      </div>
    </div>
  </div>

  <div class='container'>
    <div class='cell'>Number of Files</div>
    <div class='cell'>%NUMFILES%</div>
  </div>

  <div class='container'>
    <div class='cell'>WiFi local IP</div>
    <div class='cell'>%IP%</div>
  </div>

  <div class='container'>
    <div class='cell'>WiFi RSSI</div>
    <div class='cell'>%RSSI% dBm</div>
  </div>

  <div class='container'>
    <div class='cell'>Uptime</div>
    <div class='cell'>%UPTIME%</div>
   </div>

   <div class='container'>
    <div class='cell'>System Control</div>
    <div class='cell'>
      <form method='POST' action='/reset'>
        <input type='submit' value='Restart'>
      </form>
    </div>
   </div>

</div>
)rawliteral";

#endif