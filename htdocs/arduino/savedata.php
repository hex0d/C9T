<?php

	include("connect.php");
	
	
	
	$flag = $_GET['flag'];
	$ra = $_GET['ra'];
	$cred = $_GET['cred'];
	
	
	$sql_insert = "insert into tabelacred (flag,ra,cred) values ('$flag','$ra','$cred')";
	
	mysqli_query($connect,$sql_insert);
	
	if($sql_insert){
		echo "sucesso salva";
	}
	else{
		echo "salvo errado";
		
	}
	


?>