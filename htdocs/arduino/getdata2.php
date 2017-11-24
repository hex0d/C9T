<!DOCTYPE html>
	
	
	<html>
	<head>
		<title>ESP8266 LED Control</title>
	</head>
	<body>
	
	<!-- in the <button> tags below the ID attribute is the value sent to the arduino -->
	
	<button id="11" class="led">Toggle Pin 11</button> <!-- button for pin 11 -->
	<button id="12" class="led">Toggle Pin 12</button> <!-- button for pin 12 -->
	<button id="13" class="led">Toggle Pin 13</button> <!-- button for pin 13 -->
	<?php
	include("connect.php");
	$getra = $_GET['ra'];
	?>
	<script src="jquery.min.js"></script>
	<script type="text/javascript">
		$(document).ready(function(){
			$(".led").click(function(){
				var p = $(this).attr('ra'); // get id value (i.e. pin13, pin12, or pin11)
				// send HTTP GET request to the IP address with the parameter "pin" and value "p", then execute the function
				$.get("http://192.168.4.1:6868/", {pin:p}); // execute get request
			});
		});
	</script>
	</body>
	</html>
	



