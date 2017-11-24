<?php
	include("connect.php");
	$getra = $_GET['ra'];
	$sql = "SELECT * FROM tabelacred WHERE ra =".$getra." AND flag = 1";
	$result = mysqli_query($connect, $sql);

	if (mysqli_num_rows($result) > 0) {
		// output data of each row
    while($row = mysqli_fetch_assoc($result)) {
        echo "RA: ".$row["ra"]."@". $row["cred"];
		$sql2 = "UPDATE tabelacred SET flag = 0 WHERE id = ".$row["id"];
		mysqli_query($connect, $sql2);
    }
	} else {
		echo "0 results";
	}
	//$sql2 = "UPDATE tabelacred SET flag = 0 WHERE id = "$row["id"];


mysqli_close($connect);
	
	
	
	
	
	/*$flag = $_GET['flag'];
	$ra = $_GET['ra'];
	$cred = $_GET['cred'];
	
	
	$sql_insert = "insert into tabelacred (flag,ra,cred) values ('$flag','$ra','$cred')";
	
	mysqli_query($connect,$sql_insert);
	
	if($sql_insert){
		echo "sucesso salva";
	}
	else{
		echo "salvo errado";
		
	}*/
	


?>