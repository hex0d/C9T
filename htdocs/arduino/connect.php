<?php

	$user = "root";
	$pass = "";
	$host = "localhost";
	
	$connect = mysqli_connect($host,$user,$pass);
	$godb = mysqli_select_db($connect,'arduino');
	
	
	if($connect){
		//echo "funfo";
	}
	else {
		echo "falha";
	}




?>