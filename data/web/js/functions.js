function getXhr(){
	var xhr = null;
	if(window.XMLHttpRequest) // Firefox et autres
	   xhr = new XMLHttpRequest();
	else if(window.ActiveXObject){ // Internet Explorer
	   try {
				xhr = new ActiveXObject("Msxml2.XMLHTTP");
			} catch (e) {
				xhr = new ActiveXObject("Microsoft.XMLHTTP");
			}
	}
	else { // XMLHttpRequest non supporté par le navigateur
	   alert("Votre navigateur ne supporte pas les objets XMLHTTPRequest...");
	   xhr = false;
	}
	return xhr;
}

function readfile(file,rep)
{
	var xhr = getXhr();
	xhr.onreadystatechange = function(){
		if(xhr.readyState == 4 ){
			leselect = xhr.responseText;
			document.getElementById("title").innerHTML=file;
			document.getElementById("filename").value=file;
			document.getElementById("file").innerHTML=leselect;
		}
	}

	xhr.open("GET","readFile?rep="+escape(rep)+"&file="+escape(file),true);

	xhr.setRequestHeader('Content-Type','application/html');
	xhr.send();
}

function logRefresh()
{
	var xhr = getXhr();
	xhr.onreadystatechange = function(){
		if(xhr.readyState == 4 ){
			leselect = xhr.responseText;
			document.getElementById("console").value=leselect;
			setTimeout(function(){ logRefresh(); }, 5000);
		}
	}
	xhr.open("GET","getLogBuffer",true);
	xhr.setRequestHeader('Content-Type','application/html');
	xhr.send();
}

function scanNetwork()
{
	document.getElementById("networks").innerHTML="Scanning...";
	var xhr = getXhr();
	xhr.onreadystatechange = function(){
		if(xhr.readyState == 4 && xhr.status == 200){
			setTimeout(function(){ scanPoll(); }, 3000);
		}else if(xhr.readyState == 4){
			document.getElementById("networks").innerHTML="Scan failed";
		}
	}
	xhr.open("GET","scanStart",true);
	xhr.send();
}

function scanPoll()
{
	var xhr = getXhr();
	xhr.onreadystatechange = function(){
		if(xhr.readyState == 4 && xhr.status == 200){
			var data = JSON.parse(xhr.responseText);
			if(data.status === "done"){
				if(data.networks.length === 0){
					document.getElementById("networks").innerHTML="No networks found";
				}else{
					var html = "<select class='form-select mt-2' name='WIFISSID' onChange='updateSSID(this.value);'>";
					html += "<option value=''>-- Choose SSID --</option>";
					for(var i = 0; i < data.networks.length; i++){
						html += "<option value='" + data.networks[i].ssid + "'>";
						html += data.networks[i].ssid + " (" + data.networks[i].rssi + " dBm)</option>";
					}
					html += "</select>";
					document.getElementById("networks").innerHTML = html;
				}
			}else{
				setTimeout(function(){ scanPoll(); }, 2000);
			}
		}else if(xhr.readyState == 4){
			setTimeout(function(){ scanPoll(); }, 2000);
		}
	}
	xhr.open("GET","scanResult",true);
	xhr.send();
}

function updateSSID(val)
{
	document.getElementById("ssid").value=val;
}

function cmd(val,param="")
{

	var xhr = getXhr();
	xhr.open("GET","cmd"+val+"?param="+escape(param),true);
	xhr.setRequestHeader('Content-Type','application/html');
	xhr.send();
}

function getLatestReleaseInfo() {
	$.getJSON("https://api.github.com/repos/fairecasoimeme/LiXee-ZiBridge/releases/latest").done(function(release) {
	  var downloadCount = 0;
	  for (var i = 0; i < release.assets.length; i++) {
		downloadCount += release.assets[i].download_count;
	  }
	  var oneHour = 60 * 60 * 1000;
	  var oneDay = 24 * oneHour;
	  var dateDiff = new Date() - new Date(release.published_at);
	  var timeAgo;
	  if (dateDiff < oneDay) {
		timeAgo = (dateDiff / oneHour).toFixed(1) + " hours ago";
	  } else {
		timeAgo = (dateDiff / oneDay).toFixed(1) + " days ago";
	  }

	  var releaseInfo = release.name + " was updated " + timeAgo + " and downloaded " + downloadCount.toLocaleString() + " times.";
	  $("#releasehead").text(releaseInfo);
	  $("#releasebody").text(release.body);
	  $("#releaseinfo").fadeIn("slow");
	});
}
