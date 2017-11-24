<!DOCTYPE html>
<html>

	<head>
		<meta charset="utf-8">
		<title>FUNCIONA MERDA</title>
	</head>


	<body>
		<h1> Vamos: </h1>
		<table width = "500" border = "1" cellspacing = "2" cellpadding = "5">
			<tr>
				<td><b>ID</b></td>
				<td><b>DATA</b></td>
				<td><b>FLAG</b></td>
				<td><b>RA</b></td>
				<td><b>CRED</b></td>
			</tr>
<?php
	include("connect.php");
	$result = mysqli_query($connect,"select * from tabelacred");
	while($linha = mysqli_fetch_array($result)){
		echo'<tr>';
				echo'<td>'.$linha["id"].'</td>';
				echo'<td>'.$linha["evento"].'</td>';
				echo'<td>'.$linha["flag"].'</td>';
				echo'<td>'.$linha["ra"].'</td>';
				echo'<td>'.$linha["cred"].'</td>';
			echo'</tr>';
	}

?>
		
		</table>
	</body>

</html>