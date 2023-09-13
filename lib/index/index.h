#include <Arduino.h>
const char config_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html>
<head>
<title>Wi-Fi Realy Board</title>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
* {
  box-sizing: border-box;
}
body {
  font-family: Arial, Helvetica, sans-serif;
}
.button {
  background-color: #20a1c4;
  border: none;
  color: white;
  padding: 15px 32px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 16px;
  font-family: Roboto;
  margin: 4px 2px;
  cursor: pointer;
  border-radius: 2px;
}

.button:hover {
  background-color: #1092b6;
  color: white;
}

header {
  background-color: #1111;
  padding: 30px;
  text-align: center;
  font-size: 35px;
  font-family: verdana;
  color: #00718f;
}
nav {
  float: bottom;
  width: 100%;
  height: 100%; 
  background: #ffff;
  padding: 20px;
  text-align: center;
}
nav ul {
  list-style-type: none;
  padding: 0;
}
article {
  float: left;
  padding: 20px;
  width: 100%;
  background-color: #014d67;
  height: auto;
  color: #ffff;
  font-family: verdana;
}
section::after {
  content: "";
  display: table;
  clear: both;
} 
footer {
  background-color: #1111;
  padding: 1px;
  text-align: center;
  color: #00718f;
} 
@media (max-width: 100%) {
   article {
    width: 100%;
    height: auto;
  }
}
h3{ 
  color: #00718f;
}

fieldset {
  background-color: #014d67;
  margin: 5px;
  font-family: verdana;
  border-color: white;
  border-style: solid;
}

legend {
  background-color: #20a1c4;
  color: #ffff;
  padding: 5px 10px;
  font-family: verdana;
}

input[type=text], select {
  width: 100%;
  padding: 5px 10px;
  margin: 8px 0;
  display: inline-block;
  border: 1px solid #ccc;
  border-radius: 4px;
  box-sizing: border-box;
  }

</style>
</head>
<body> 

<header>
<h2>Wi-Fi Relay Board Configuration</h2>
</header>
<section>
	<article>
    	<fieldset>
        <legend>Firmware:</legend> 
            <table>
              <tr> 
              	<td>Version:</td>
                <td><input type="text" id="fV" readonly></td> 
              </tr>  
            </table> 
        </fieldset>
        <br>
        <form>
            <fieldset>
            <legend>MQTT Details:</legend> 
            <table>
              <tr> 
                  <td>MQTT Server:</td>
                  <td><input type="text" id="mS"></td>
                  <td>MQTT Port:</td>
                  <td><input type="text" id="mP"> </td>
              </tr>  
            </table>
            </fieldset>
      	</form> 
        <br>
        <form>
          <fieldset>
          <legend>Network:</legend>  
            <table>
              <tr>
                  <td>Static IP Enabled:</td>
                  <td><input type="checkbox" id="sIPs" name="staticIPState" value="staticIPState"></td>

              </tr>
              <tr> 
                  <td>Static IP:</td>
                  <td><input type="text" id="sIP" ></td>

              <tr> 
                  <td>Subnet:</td>
                  <td><input type="text" id="sN" ></td>
              </tr>
              <tr> 
                  <td>Default Gateway:</td>
                  <td><input type="text" id="gW" ></td>
              </tr>
              <tr> 
                  <td>Primary DNS:</td>
                  <td><input type="text" id="pD" ></td>
                  <td>Secondary DNS:</td>
                  <td><input type="text" id="sD" ></td>
              </tr>
            </table>
          </fieldset>
      	</form> 
        <br>
        <form>
          <fieldset>
          <legend>Device Information:</legend>  
            <table>
              <tr> 
                  <td>Device ID:</td>
                  <td><input type="text" id="dI"></td>
                  <td>Company ID:</td>
                  <td><input type="text" id="cI"> </td>
                  <td>Location ID:</td>
                  <td><input type="text" id="lI"> </td>
              </tr> 
            </table>
          </fieldset>
    	</form>
      	<br>
        <form>
          <fieldset>
          <legend>Wi-Fi Credentials:</legend>  
            <table> 
              <tr> 
                  <td>SSID[0]:</td>
                  <td><input type="text" id="s0"></td>
                  <td>PASS[0]:</td>
                  <td><input type="text" id="p0"> </td>
              </tr>  
              <tr> 
                  <td>SSID[1]:</td>
                  <td><input type="text" id="s1"></td>
                  <td>PASS[1]:</td>
                  <td><input type="text" id="p1"> </td>
              </tr>  
              <tr> 
                  <td>SSID[2]:</td>
                  <td><input type="text" id="s2"></td>
                  <td>PASS[2]:</td>
                  <td><input type="text" id="p2"> </td>
              </tr> 
              <tr> 
                  <td>SSID[3]:</td>
                  <td><input type="text" id="s3"></td>
                  <td>PASS[3]:</td>
                  <td><input type="text" id="p3"> </td>
              </tr>  
              <tr> 
                  <td>SSID[4]:</td>
                  <td><input type="text" id="s4"></td>
                  <td>PASS[4]:</td>
                  <td><input type="text" id="p4"> </td>
              </tr>  
            </table>
          </fieldset>
      </form> 
      <br>
      <form>
      <fieldset>
      <legend>Relay Configuration:</legend>  
      	<br>
      	<fieldset>
        <legend>Group Code:</legend>  
          <table>
            <tr> 
                <td>Green:</td>
                <td><input type="text" id="ggc"></td>
                <td>Orange:</td>
                <td><input type="text" id="ogc"> </td>
                <td>Red:</td>
                <td><input type="text" id="rgc"> </td>
                <td>Cancel:</td>
                <td><input type="text" id="cgc"> </td>
            </tr> 
          </table> 
        </fieldset>
        <br>
      	<fieldset>
        <legend>Off Time:</legend>  
          <table>
            <tr> 
                <td>Green:</td>
                <td><input type="text" id="got"></td>
                <td>Orange:</td>
                <td><input type="text" id="oot"> </td>
                <td>Red:</td>
                <td><input type="text" id="rot"> </td>
                <td>Cancel:</td>
                <td><input type="text" id="cot"> </td>
            </tr> 
          </table>
        </fieldset>
        <br>
      	<fieldset>
        <legend>Active State:</legend>  
          <h1>NC = 1, NO = 0</h1>
          <table>
            <tr> 
                <td>STATE:</td>
                <td><input type="text" id="ros"></td>
            </tr> 
          </table>
      	</fieldset>
      <br>
      </fieldset>
      </form>
</article>
</section>
 <nav>
     <button onclick="config()" value="Configure" id="config" style="height: 50px; width: 200px" class="button">Apply Settings</button>
     </nav>
<footer> 

<p>© 2020 Security Communication Solutions International</p>
</footer>
	<script>
    
    function config(){  
		var myStr = "/get?program=" 
        
        myStr += "staips=" + ((document.getElementById('sIPs').checked) ? 1 : 0) +"?"; 
        
		if(document.getElementById('mS').value != "")
		{
			myStr += "mqttip="+document.getElementById('mS').value+"?";
		}
		
		if(document.getElementById('mP').value != "")
		{
			myStr += "mqtt_p="+document.getElementById('mP').value+"?";
		} 
		
        if(document.getElementById('sIP').value != "")
		{
			myStr += "sta_ip="+document.getElementById('sIP').value+"?";
		}
		
		if(document.getElementById('sN').value != "")
		{
			myStr += "subnet="+document.getElementById('sN').value+"?";
		}
		
		if(document.getElementById('gW').value != "")
		{
			myStr += "gate_w="+document.getElementById('gW').value+"?";
		}
		
		if(document.getElementById('pD').value != "")
		{
			myStr += "p_dns="+document.getElementById('pD').value+"?";
		}
		
		if(document.getElementById('sD').value != "")
		{
			myStr += "s_dns="+document.getElementById('sD').value+"?";
		} 
		if(document.getElementById('dI').value != "")
		{
			myStr += "wa_idn="+document.getElementById('dI').value+"?";
		} 
		
		if(document.getElementById('cI').value != "")
		{
			myStr += "co_idn="+document.getElementById('cI').value+"?";
		} 
		
        if(document.getElementById('lI').value != "")
		{
			myStr += "lo_idn="+document.getElementById('lI').value+"?";
		} 

		if(document.getElementById('s0').value != "")
		{
			myStr += "ssid_0="+document.getElementById('s0').value+"?";
		} 
		
		if(document.getElementById('s1').value != "")
		{
			myStr += "ssid_1="+document.getElementById('s1').value+"?";
		} 
		if(document.getElementById('s2').value != "")
		{
			myStr += "ssid_2="+document.getElementById('s2').value+"?";
		} 
		if(document.getElementById('s3').value != "")
		{
			myStr += "ssid_3="+document.getElementById('s3').value+"?";
		} 
		if(document.getElementById('s4').value != "")
		{
			myStr += "ssid_4="+document.getElementById('s4').value+"?";
		} 
		
		if(document.getElementById('p0').value != "")
		{
			myStr += "pass_0="+document.getElementById('p0').value+"?";
		} 
		
		if(document.getElementById('p1').value != "")
		{
			myStr += "pass_1="+document.getElementById('p1').value+"?";
		} 
		
		if(document.getElementById('p2').value != "")
		{
			myStr += "pass_2="+document.getElementById('p2').value+"?";
		} 
		
		if(document.getElementById('p3').value != "")
		{
			myStr += "pass_3="+document.getElementById('p3').value+"?";
		} 
		
		if(document.getElementById('p4').value != "")
		{
			myStr += "pass_4="+document.getElementById('p4').value+"?";
		} 
		
		if(document.getElementById('ggc').value != "")
		{
			myStr += "grn_gc="+document.getElementById('ggc').value+"?";
		} 
		
		if(document.getElementById('ogc').value != "")
		{
			myStr += "org_gc="+document.getElementById('ogc').value+"?";
		} 
		
		if(document.getElementById('rgc').value != "")
		{
			myStr += "red_gc="+document.getElementById('rgc').value+"?";
		} 
		
		if(document.getElementById('cgc').value != "")
		{
			myStr += "can_gc="+document.getElementById('cgc').value+"?";
		} 
		
		if(document.getElementById('got').value != "")
		{
			myStr += "grn_ot="+document.getElementById('got').value+"?";
		} 
		
		if(document.getElementById('oot').value != "")
		{
			myStr += "org_ot="+document.getElementById('oot').value+"?";
		} 
		
		if(document.getElementById('rot').value != "")
		{
			myStr += "red_ot="+document.getElementById('rot').value+"?";
		} 
		
		if(document.getElementById('cot').value != "")
		{
			myStr += "can_ot="+document.getElementById('cot').value+"?";
		} 
		
		if(document.getElementById('ros').value != "")
		{
			myStr += "rly_os="+document.getElementById('ros').value+"?";
		} 

    var newStr = myStr.replace(/\+/g, "%2B");
    console.log(newStr);
		window.location.href = newStr;
    alert("The device will restart now.");
    } 
    getData();
    function getData() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == XMLHttpRequest.DONE && this.status == 200) {
          var str = this.responseText;
          var res = str.split("|"); 
          document.getElementById('mS').value = res[0];  
          document.getElementById('mP').value = res[1];  
          document.getElementById('dI').value = res[2];
          document.getElementById('cI').value = res[3];
          document.getElementById('lI').value = res[4];
          document.getElementById('s0').value = res[5];
          document.getElementById('s1').value = res[6];
          document.getElementById('s2').value = res[7];
          document.getElementById('s3').value = res[8];
          document.getElementById('s4').value = res[9];
          document.getElementById('p0').value = res[10];
          document.getElementById('p1').value = res[11];
          document.getElementById('p2').value = res[12];
          document.getElementById('p3').value = res[13];
          document.getElementById('p4').value = res[14];
          document.getElementById('ggc').value = res[15];
          document.getElementById('ogc').value = res[16];
          document.getElementById('rgc').value = res[17];
          document.getElementById('cgc').value = res[18];
          document.getElementById('got').value = res[19];
          document.getElementById('oot').value = res[20];
          document.getElementById('rot').value = res[21];
          document.getElementById('cot').value = res[22];
          document.getElementById('ros').value = res[23]; 
          document.getElementById('fV').value = res[24];
          document.getElementById('sIP').value = res[25];
          document.getElementById('sN').value = res[26];
          document.getElementById('gW').value = res[27];
          document.getElementById('pD').value = res[28];
          document.getElementById('sD').value = res[29];
          document.getElementById('sIPs').checked = ((res[30] == '1') ? true : false);
          console.log(this.responseText);
          
        }
      };
      xhttp.open("GET", "configuration", true);
      xhttp.send();
    } 
    </script>
</body></html>)rawliteral";

#if 0
const char config_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html>
	<head>
  	<title>Wi-Fi Relay Board Configuration</title>
  	<meta name="viewport" content="width=device-width, initial-scale=1"> 
  	</head><body>
  	<h2>Wi-Fi Relay Board Configuration.</h2>
      <h3>FIRMWARE</h3>
      <table>
        <tr> 
        	<td>Version:</td>
          <td><input type="text" id="fV" readonly></td> 
        </tr>  
      </table> 
      <h3>MQTT DETAILS</h3>
      <table>
        <tr> 
            <td>MQTT Server:</td>
            <td><input type="text" id="mS"></td>
            <td>MQTT Port:</td>
            <td><input type="text" id="mP"> </td>
        </tr>  
      </table>
      <h3>NETWORK</h3>
      <table>
        <tr>
            <td>Static IP Enabled:</td>
            <td><input type="checkbox" id="sIPs" name="staticIPState" value="staticIPState"></td>
            
        </tr>
        <tr> 
      		<td>Static IP:</td>
            <td><input type="text" id="sIP" ></td>
            
        <tr> 
      		<td>Subnet:</td>
            <td><input type="text" id="sN" ></td>
        </tr>
        <tr> 
      		<td>Default Gateway:</td>
            <td><input type="text" id="gW" ></td>
        </tr>
        <tr> 
      		<td>Primary DNS:</td>
            <td><input type="text" id="pD" ></td>
            <td>Secondary DNS:</td>
            <td><input type="text" id="sD" ></td>
        </tr>
      </table>
      <h3>DEVICE ID's</h3> 
      <table>
      	<tr> 
            <td>Device ID:</td>
            <td><input type="text" id="dI"></td>
            <td>Company ID:</td>
            <td><input type="text" id="cI"> </td>
            <td>Location ID:</td>
            <td><input type="text" id="lI"> </td>
        </tr> 
      </table>
      <h3>WIFI CREDENTIALS</h3>
      <table> 
        <tr> 
            <td>SSID[0]:</td>
            <td><input type="text" id="s0"></td>
            <td>PASS[0]:</td>
            <td><input type="text" id="p0"> </td>
        </tr>  
        <tr> 
            <td>SSID[1]:</td>
            <td><input type="text" id="s1"></td>
            <td>PASS[1]:</td>
            <td><input type="text" id="p1"> </td>
        </tr>  
        <tr> 
            <td>SSID[2]:</td>
            <td><input type="text" id="s2"></td>
            <td>PASS[2]:</td>
            <td><input type="text" id="p2"> </td>
        </tr> 
        <tr> 
            <td>SSID[3]:</td>
            <td><input type="text" id="s3"></td>
            <td>PASS[3]:</td>
            <td><input type="text" id="p3"> </td>
        </tr>  
        <tr> 
            <td>SSID[4]:</td>
            <td><input type="text" id="s4"></td>
            <td>PASS[4]:</td>
            <td><input type="text" id="p4"> </td>
        </tr>  
      </table>
      <h3>GROUP CODE</h3>
      <table>
      	<tr> 
            <td>Green:</td>
            <td><input type="text" id="ggc"></td>
            <td>Orange:</td>
            <td><input type="text" id="ogc"> </td>
            <td>Red:</td>
            <td><input type="text" id="rgc"> </td>
            <td>Cancel:</td>
            <td><input type="text" id="cgc"> </td>
        </tr> 
      </table>
      <h3>RELAY OFF TIME (seconds)</h3>
      <table>
      	<tr> 
            <td>Green:</td>
            <td><input type="text" id="got"></td>
            <td>Orange:</td>
            <td><input type="text" id="oot"> </td>
            <td>Red:</td>
            <td><input type="text" id="rot"> </td>
            <td>Cancel:</td>
            <td><input type="text" id="cot"> </td>
        </tr> 
      </table>
      <h3>RELAY ON STATE (1 = NC, 0 = NO) </h3>
      <table>
      	<tr> 
            <td>STATE:</td>
            <td><input type="text" id="ros"></td>
        </tr> 
      </table>
      <br>
      <button onclick="config()" value="Configure" id="config" style="height: 50px; width: 200px">CONFIGURE</button>
    
	<script>
    
    function config(){  
		var myStr = "/get?program=" 
        
        myStr += "staips=" + ((document.getElementById('sIPs').checked) ? 1 : 0) +"?"; 
        
		if(document.getElementById('mS').value != "")
		{
			myStr += "mqttip="+document.getElementById('mS').value+"?";
		}
		
		if(document.getElementById('mP').value != "")
		{
			myStr += "mqtt_p="+document.getElementById('mP').value+"?";
		} 
		
        if(document.getElementById('sIP').value != "")
		{
			myStr += "sta_ip="+document.getElementById('sIP').value+"?";
		}
		
		if(document.getElementById('sN').value != "")
		{
			myStr += "subnet="+document.getElementById('sN').value+"?";
		}
		
		if(document.getElementById('gW').value != "")
		{
			myStr += "gate_w="+document.getElementById('gW').value+"?";
		}
		
		if(document.getElementById('pD').value != "")
		{
			myStr += "p_dns="+document.getElementById('pD').value+"?";
		}
		
		if(document.getElementById('sD').value != "")
		{
			myStr += "s_dns="+document.getElementById('sD').value+"?";
		} 
		if(document.getElementById('dI').value != "")
		{
			myStr += "wa_idn="+document.getElementById('dI').value+"?";
		} 
		
		if(document.getElementById('cI').value != "")
		{
			myStr += "co_idn="+document.getElementById('cI').value+"?";
		} 
		
        if(document.getElementById('lI').value != "")
		{
			myStr += "lo_idn="+document.getElementById('lI').value+"?";
		} 

		if(document.getElementById('s0').value != "")
		{
			myStr += "ssid_0="+document.getElementById('s0').value+"?";
		} 
		
		if(document.getElementById('s1').value != "")
		{
			myStr += "ssid_1="+document.getElementById('s1').value+"?";
		} 
		if(document.getElementById('s2').value != "")
		{
			myStr += "ssid_2="+document.getElementById('s2').value+"?";
		} 
		if(document.getElementById('s3').value != "")
		{
			myStr += "ssid_3="+document.getElementById('s3').value+"?";
		} 
		if(document.getElementById('s4').value != "")
		{
			myStr += "ssid_4="+document.getElementById('s4').value+"?";
		} 
		
		if(document.getElementById('p0').value != "")
		{
			myStr += "pass_0="+document.getElementById('p0').value+"?";
		} 
		
		if(document.getElementById('p1').value != "")
		{
			myStr += "pass_1="+document.getElementById('p1').value+"?";
		} 
		
		if(document.getElementById('p2').value != "")
		{
			myStr += "pass_2="+document.getElementById('p2').value+"?";
		} 
		
		if(document.getElementById('p3').value != "")
		{
			myStr += "pass_3="+document.getElementById('p3').value+"?";
		} 
		
		if(document.getElementById('p4').value != "")
		{
			myStr += "pass_4="+document.getElementById('p4').value+"?";
		} 
		
		if(document.getElementById('ggc').value != "")
		{
			myStr += "grn_gc="+document.getElementById('ggc').value+"?";
		} 
		
		if(document.getElementById('ogc').value != "")
		{
			myStr += "org_gc="+document.getElementById('ogc').value+"?";
		} 
		
		if(document.getElementById('rgc').value != "")
		{
			myStr += "red_gc="+document.getElementById('rgc').value+"?";
		} 
		
		if(document.getElementById('cgc').value != "")
		{
			myStr += "can_gc="+document.getElementById('cgc').value+"?";
		} 
		
		if(document.getElementById('got').value != "")
		{
			myStr += "grn_ot="+document.getElementById('got').value+"?";
		} 
		
		if(document.getElementById('oot').value != "")
		{
			myStr += "org_ot="+document.getElementById('oot').value+"?";
		} 
		
		if(document.getElementById('rot').value != "")
		{
			myStr += "red_ot="+document.getElementById('rot').value+"?";
		} 
		
		if(document.getElementById('cot').value != "")
		{
			myStr += "can_ot="+document.getElementById('cot').value+"?";
		} 
		
		if(document.getElementById('ros').value != "")
		{
			myStr += "rly_os="+document.getElementById('ros').value+"?";
		} 

    var newStr = myStr.replace(/\+/g, "%2B");
    console.log(newStr);
		window.location.href = newStr;
    alert("The device will restart now.");
    } 
    getData();
    function getData() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == XMLHttpRequest.DONE && this.status == 200) {
          var str = this.responseText;
          var res = str.split("|"); 
          document.getElementById('mS').value = res[0];  
          document.getElementById('mP').value = res[1];  
          document.getElementById('dI').value = res[2];
          document.getElementById('cI').value = res[3];
          document.getElementById('lI').value = res[4];
          document.getElementById('s0').value = res[5];
          document.getElementById('s1').value = res[6];
          document.getElementById('s2').value = res[7];
          document.getElementById('s3').value = res[8];
          document.getElementById('s4').value = res[9];
          document.getElementById('p0').value = res[10];
          document.getElementById('p1').value = res[11];
          document.getElementById('p2').value = res[12];
          document.getElementById('p3').value = res[13];
          document.getElementById('p4').value = res[14];
          document.getElementById('ggc').value = res[15];
          document.getElementById('ogc').value = res[16];
          document.getElementById('rgc').value = res[17];
          document.getElementById('cgc').value = res[18];
          document.getElementById('got').value = res[19];
          document.getElementById('oot').value = res[20];
          document.getElementById('rot').value = res[21];
          document.getElementById('cot').value = res[22];
          document.getElementById('ros').value = res[23]; 
          document.getElementById('fV').value = res[24];
          document.getElementById('sIP').value = res[25];
          document.getElementById('sN').value = res[26];
          document.getElementById('gW').value = res[27];
          document.getElementById('pD').value = res[28];
          document.getElementById('sD').value = res[29];
          document.getElementById('sIPs').checked = ((res[30] == '1') ? true : false);
          console.log(this.responseText);
          
        }
      };
      xhttp.open("GET", "configuration", true);
      xhttp.send();
    } 
    </script>
</body></html>)rawliteral";
#endif

const char indexPage[] PROGMEM = R"=====(

<!DOCTYPE html>
  <html>
  <head>
  <title>Wi-Fi Relay Board v1.0</title>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>
  <style>
  html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #444444;}
  body{margin: 0px;} 
  h1 {margin: 50px auto 30px;} 
  .side-by-side{display: table-cell;vertical-align: middle;position: relative;}
  .text{font-weight: 600;font-size: 19px;width: 200px;}
  .reading{font-weight: 300;font-size: 50px;padding-right: 25px;}
  .temperature .reading{color: #F29C1F;}
  .humidity .reading{color: #3B97D3;}
  .pressure .reading{color: #26B99A;}
  .altitude .reading{color: #955BA5;}
  .time .reading{color: #16A085;}
  .date .reading{color: #16A085;}
  .superscript{font-size: 17px;font-weight: 600;position: absolute;top: 10px;}
  .data{padding: 10px;}
  .container{display: table;margin: 0 auto;}
  .icon{width:65px}
  </style>
  </head>
  <body>
  <h1>Wi-Fi Relay Board v1.0</h1> 
  
  
  <div class='container'>
  <div class='Logo'>
  <div class='side-by-side icon'>
  <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 48 48" width="100px" height="100px"><path fill="none" stroke="#59b4d9" stroke-linecap="round" stroke-linejoin="round" stroke-miterlimit="10" stroke-width="4" d="M15,12h-5c-2.761,0-5,2.239-5,5v14c0,2.761,2.239,5,5,5h7"/><path fill="none" stroke="#804998" stroke-linecap="round" stroke-linejoin="round" stroke-miterlimit="10" stroke-width="4" d="M34,36h4c2.761,0,5-2.239,5-5V17c0-2.761-2.239-5-5-5h-7"/><path fill="#7eba00" d="M24 9A3 3 0 1 0 24 15 3 3 0 1 0 24 9zM24 33A3 3 0 1 0 24 39 3 3 0 1 0 24 33z"/><path fill="none" stroke="#68217a" stroke-linecap="round" stroke-linejoin="round" stroke-miterlimit="10" stroke-width="4" d="M18.595,16.422C19.878,17.99,21.816,19,24,19c3.866,0,7-3.134,7-7s-3.134-7-7-7c-2.184,0-4.122,1.01-5.405,2.578"/><path fill="none" stroke="#0072c6" stroke-linecap="round" stroke-linejoin="round" stroke-miterlimit="10" stroke-width="4" d="M29.405,31.578C28.122,30.01,26.184,29,24,29c-3.866,0-7,3.134-7,7s3.134,7,7,7c2.184,0,4.122-1.01,5.405-2.578"/></svg></div>
  </div>

  </body>
  </html> 
)=====";
